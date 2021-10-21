#include "TrigScint/QIEDecoder.h"

//#include <algorithm>
//#include <iomanip>
#include <bitset>


namespace trigscint {

 void QIEDecoder::configure(
	  framework::config::Parameters &ps) {
  // Configure this instance of the encoder
   outputCollection_ = ps.getParameter<std::string>("output_collection");
   inputCollection_ = ps.getParameter<std::string>("input_collection");
   inputPassName_ = ps.getParameter<std::string>("input_pass_name");
   channelMapFileName_ = ps.getParameter<std::string>("channel_map_file");
   nChannels_ = ps.getParameter<int>("number_channels");
   verbose_ = ps.getParameter<bool>("verbose");

   ldmx_log(debug) << "In configure, got parameters:" <<
	 "\noutput_collection = " << outputCollection_ <<
	 "\ninput_collection = " << inputCollection_ <<
	 "\ninput_pass_name  = " << inputPassName_ <<
	 "\nchannel_map_file = " <<  channelMapFileName_ <<
	 "\nnumber_channels  = " <<  nChannels_ <<
	 "\nverbose          = " << verbose_ ;

   channelMapFile_.open(channelMapFileName_, std::ios::in);
   if (! channelMapFile_.is_open() ){
	 EXCEPTION_RAISE("BadMapFile", 	"The channel mapping file cannot be opened."); // <-- appears this needs implementing first 
	 ldmx_log(fatal) <<	"The channel mapping file cannot be opened.";
	 return;
   }
   int chID, elID;
   while (! channelMapFile_.eof() ){
	 channelMapFile_ >> elID >> chID ;
	 // make the map based on electronics ID, to look up channel ID.
	 // the reason is, we will only know the elecID from the position
	 // of the word in the stream. so these need to be strictly ordered.
	 // writing the right bar in the right position is easier if we can just
	 // read this map from beginning to end.
	 // barID can always be set, or looked up, as a property of the digi.

	 // here make the elecID the key (other way around when encoding)
	 channelMap_.insert(std::pair<int,int>(elID, chID));
	 //	 channelMap_[ chID ] = elID;
	 ldmx_log(debug) << elID << "  chID " << chID ;
   }
   channelMapFile_.close();
   if (elID != nChannels_ - 1 )
	 ldmx_log(fatal) << "The set number of channels " << nChannels_ << " seems not to match the number from the map (+1) :" << elID; 
   return;
   
 }

  void QIEDecoder::produce(framework::Event &event) {
    ldmx_log(debug)
	  << "QIEDecoder: produce() starts! Event number: "
	  << event.getEventHeader().getEventNumber();

  std::vector <trigscint::QIEStream> qieOuts;
  int nSamp = QIEStream::NUM_SAMPLES ;
  std::vector < int > initVec(nSamp, 0);

  ldmx_log(debug) << "num samples = " << nSamp;
    
  for (int iQ = 0; iQ < nChannels_ ; iQ++) {
	QIEStream qieOut;
	qieOut.setADC(initVec);
	qieOut.setTDC(initVec);
	qieOut.setCID(initVec);
	qieOuts.push_back(qieOut);
  }

  ldmx_log(debug) << "Looking up input collection " << inputCollection_ << "_" << inputPassName_;
  const auto eventStream{event.getCollection<uint8_t>( inputCollection_, inputPassName_)};
  ldmx_log(debug) << "Got input collection" << inputCollection_ << "_" << inputPassName_;

  // trigger IDevent number
  // this is the one word that spans several bytes 
  uint16_t triggerID =0; // =eventStream.at(QIEStream::TRIGID_POS);
  
  //  for (int iW = QIEStream::TRIGID_LEN_BYTES-1; iW >= 0; iW--) { //assume the whole 2B are written as a single 16 bits word                      
	for (int iW = 0 ; iW < QIEStream::TRIGID_LEN_BYTES; iW++) { //assume the whole 2B are written as a single 16 bits word
	int pos = QIEStream::TRIGID_POS+(QIEStream::TRIGID_LEN_BYTES -1 - iW);
	uint8_t tIDword = eventStream.at( pos );
	//	uint8_t tIDword = eventStream.at(QIEStream::TRIGID_POS+iW);
	ldmx_log(debug) << "trigger word at position " << pos << " (with iW = " << iW << ") = " << std::bitset<8>(tIDword ) ;
	triggerID |= (tIDword << iW*8); //shift by a byte at a time
  }
  
  //	  flags |= (isCRC0malformed << QIEStream::CRC0_ERR_POS);
  //triggerIDwords.push_back( tIDword );

  
	  // uint16_t triggerID =eventStream.at(QIEStream::TRIGID_POS);
  ldmx_log(debug) << " got triggerID " << std::bitset<16>(triggerID) ; //eventStream.at(0);

  if ( triggerID != event.getEventHeader().getEventNumber() ) {
	// this probably only applies to digi emulation,
	// unless an event number is explicitly set in unpacking
	ldmx_log(fatal) << "Got event number mismatch: framework reports " <<
	  event.getEventHeader().getEventNumber() <<", stream says " << triggerID ;
  }
  // error word 
  /* the error word contains 
	 - 4 trailing reserved 0's, for now
	 - isCIDunsync : if there is a mismatch between CID reported by channels within the same time sample 
	 - isCIDskipped : if there is a gap in the CID increment of a channel beweeen samples 
	 - isCRC0malformed : if there was an issue with CRC from fiber0
	 - isCRC1malformed : if there was an issue with CRC from fiber1
  */
  uint8_t flags = eventStream.at(QIEStream::ERROR_POS);  

  bool isCIDskipped { (flags >> QIEStream::CID_SKIP_POS) & mask8<QIEStream::FLAG_SIZE_BITS>::m};
  bool isCIDunsync { (flags >> QIEStream::CID_UNSYNC_POS) & mask8<QIEStream::FLAG_SIZE_BITS>::m};
  bool isCRC1malformed{ (flags >> QIEStream::CRC1_ERR_POS) & mask8<QIEStream::FLAG_SIZE_BITS>::m};
  bool isCRC0malformed{ (flags >> QIEStream::CRC0_ERR_POS) & mask8<QIEStream::FLAG_SIZE_BITS>::m};
  //checksum
  uint8_t referenceChecksum = 30;  // really, this is just a random number for now. TODO> implement a checksum set/get 
  int checksum = eventStream.at(QIEStream::CHECKSUM_POS);
  if ( checksum != referenceChecksum) 
	ldmx_log(fatal) << "Got checksum mismatch: expected " << (int)referenceChecksum << ", stream says " << checksum ;
  if (isCIDunsync)
	ldmx_log(debug) << "Found unsynced CIDs!";
  if (isCIDskipped)
	ldmx_log(fatal) << "Found skipped CIDs!" ;

  /* -- TS event header done; read the channel contents -- */
  std::vector<trigscint::TrigScintQIEDigis> outDigis;
  std::map < int, std::vector<int>> ADCmap;
  std::map < int, std::vector<int>> TDCmap;
  
  // read in words from the stream. line them up per channel and time sample.
  // channels are in the electronics ordering 

  // outer loop: over nSamples
  // inner loop over nChannels to get ADCs, then repeat to get TDCs

  int iWstart = std::max( std::max(QIEStream::ERROR_POS, QIEStream::CHECKSUM_POS),
						  QIEStream::TRIGID_POS+(QIEStream::TRIGID_LEN_BYTES)) +1;  //probably overkill :D should be 4 
  ldmx_log(debug) << "Event parsing starts at vector idx " << iWstart ; 
  int nWords = nSamp*nChannels_*2 + iWstart; //1 ADC, 1 TDC per channel per sample, + the words in the header
  int iWord = iWstart;
  for (int iS = 0; iS < nSamp; iS++) {                                                                                                     
	for (int iQ = 0; iQ < nChannels_ ; iQ++) {
	  int val = eventStream.at(iWord);
	  if (val > 0) { 	//add only the digis with non-zero ADC value
		ldmx_log(debug) << "got ADC value " << val ;
		if ( ADCmap.find( iQ ) == ADCmap.end() ) { // we have a new channel 
		  std::vector <int > adcs(nSamp, 0);
		  ADCmap.insert(std::pair<int, std::vector <int> >(iQ, adcs));
		}
		ADCmap[ iQ ].at(iS) = val;
	  }
	  iWord++;
	}
	for (int iQ = 0; iQ < nChannels_ ; iQ++) {
	  int val = eventStream.at(iWord);
	  if (val > 0) { // TODO: check if this channel is also present in ADC map? in the end? 
		ldmx_log(debug) << "got TDC value " << val ;
		if ( TDCmap.find( iQ ) == TDCmap.end() ) { // we have a new channel 
		  std::vector <int > tdcs(nSamp, 0);
		  TDCmap.insert(std::pair<int, std::vector <int> >(iQ, tdcs));
		}
		//this is LETDC; only the two most significant bits included
		// they are shipped as least significant bits --> shift them 
		TDCmap[ iQ ].at(iS) = val*16;  
	  }
	  iWord++;
	}
	if ( iWord > nWords )
	  ldmx_log(fatal) << "More words than expected! ";
  }
  
  for (std::map<int, std::vector<int>>::iterator itr = ADCmap.begin(); itr != ADCmap.end(); ++itr) {
	TrigScintQIEDigis  digi;
	digi.setADC( itr->second );
	int bar = channelMap_[itr->first]; // same as .find( itr->first )->second;
	digi.setChanID( bar );
	digi.setTDC( TDCmap[itr->first] );
	outDigis.push_back( digi );
	ldmx_log(debug) << "Iterator points to key " << itr->first
					<< " and mapped channel supposedly  is " << bar ;
	ldmx_log(debug) << "Made digi with barID = " << digi.getChanID()
					<< ", third adc value " << digi.getADC().at(2)
					<< " and third tdc " << digi.getTDC().at(2) ;
  }
 
  event.add(outputCollection_, outDigis);
  }

void QIEDecoder::onFileOpen() {
  ldmx_log(debug) << "Opening file!";
  
  return;
}

void QIEDecoder::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void QIEDecoder::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void QIEDecoder::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}


}  // namespace trigscint                                                                                                                           

DECLARE_PRODUCER_NS(trigscint, QIEDecoder);

