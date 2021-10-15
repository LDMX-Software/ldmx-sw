#include "TrigScint/QIEEncoder.h"

//#include <algorithm>
#include <iomanip>
#include <bitset>

namespace trigscint {

  
 void QIEEncoder::configure(
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

   //set up channel mapping    
  channelMapFile_.open(channelMapFileName_, std::ios::in);
   if (! channelMapFile_.is_open() ){
     EXCEPTION_RAISE("BadMapFile",  "The channel mapping file cannot be opened."); // <-- appears this needs implementing first  
     ldmx_log(fatal) <<	"The channel mapping file cannot be opened.";
	 return;
   }
   int chID, elID;
   while (! channelMapFile_.eof() ){
	 channelMapFile_ >> elID >> chID ;
	 // for test beam, we will only know the elecID from
	 // the position of the word in the stream.
	 // so these need to be strictly ordered in the map.
	 // barID can always be set, or looked up, as a property of the digi.

	 //make this based on channel ID. this is like looking up the position in a vector of a certain value. but it's fine 
	 channelMap_.insert(std::pair<int,int>(chID, elID));
	 ldmx_log(debug) << "elID " << elID << "  chID " << chID ;
   }
   if (elID != nChannels_ - 1 )
     ldmx_log(fatal) << "The set number of channels " << nChannels_ << " seems not to match the number from the map (+1) :" << elID;
   channelMapFile_.close();
   
   return;
 }
  
  
  void QIEEncoder::produce(framework::Event &event) {
    ldmx_log(debug)
	  << "QIEEncoder: produce() starts! Event number: "
	  << event.getEventHeader().getEventNumber();
	
    std::vector <trigscint::QIEStream> qieOuts;
	int nSamp = QIEStream::NUM_SAMPLES ;
	std::vector < int > initVec(nSamp, 0);
	ldmx_log(debug) << "num samples = " << nSamp;
	
	//we're keeping a list ordered in elec ID since this is the order we'll use to write them to stream
	for (int iQ = 0; iQ < nChannels_ ; iQ++) {
	  QIEStream qieOut;
	  qieOut.setADC(initVec);
	  qieOut.setTDC(initVec);
	  qieOut.setCID(initVec);
	  qieOut.setElectronicsID( iQ ); //assume id is index //channelMap_.at(iQ) );
	
	  qieOuts.push_back(qieOut);
	}
	
	ldmx_log(debug) << "Looking up input collection " << inputCollection_ << "_" << inputPassName_;
	const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
																	   inputCollection_, inputPassName_)};
	ldmx_log(debug) << "Got input collection" << inputCollection_ << "_" << inputPassName_;

	bool isCIDunsync = false;   //if there is a mismatch between CID reported by channels within the same time sample 
	bool isCIDskipped = false;  //if there is a gap in the CID increment. 
	bool isCRC0malformed = false; //if there was an issue with CRC from fiber0
	bool isCRC1malformed = false; //if there was an issue with CRC from fiber1

	int firstCID=-1;
	ldmx_log(debug) << "entering loop over digis " ;
	for (auto &digi: digis) {
	  int bar = digi.getChanID();
	  auto itr = channelMap_.find( bar );
	  if ( itr == channelMap_.end() ) {// yikes! didn't find the bar in the map
		ldmx_log(fatal) << "Couldn't find an entry for bar " << bar << "; check the (choice of) channel map!. Exiting event " << event.getEventHeader().getEventNumber();
		return;
	  }
	  int idx = itr->second; //here we're just using the order. no actual elID is assumed.
	  qieOuts.at(idx).setChannelID( bar );
	  qieOuts.at(idx).setElectronicsID( idx ); 
	  //	ldmx_log(debug) << "Channel "<< bar << " elec ID " << unsigned(qieOuts.at(idx).getElectronicsID()) ;
	  ldmx_log(debug) << "Channel "<< bar << " elec ID " << qieOuts.at(idx).getElectronicsID() ;
	  std::vector <int> LEtdcs; //make the LE (Leading Edge) truncation explicit 
	  std::vector <uint8_t> cids;
	  for (int iS = 0; iS < nSamp; iS++) {
		int tdc= digi.getTDC().at(iS);
		int cid= digi.getCID().at(iS);
		if ( cids.size()>0 && (cid%4)!=( (cids.back()+1)%4) ) {
		  //by construction shouldn't happen in simulation. still, explicitly
		  //checking here, considering any future changes to our CID simulation. 
		  isCIDskipped = true;
		}
		if (verbose_) { //all this is only useful for debugging 
		  std::vector <uint8_t> adcs;
		  int adc= digi.getADC().at(iS);
		  uint8_t mant = adc%64 ;
		  uint8_t exp = adc/64 ;
		  ldmx_log(debug) << "\tSample " << iS  << std::left << std::setw(6) << " ADC " << adc << ",\texp " << unsigned(exp) << " mant " << unsigned(mant) <<
			",\tTDC = " << tdc << ", LE TDC = " << std::bitset<8>(tdc/16) << " and capID= " << cid ; // << std::endl;
		  adcs.push_back( 64*exp+mant);
		  ldmx_log(debug)  << "Combined ADC: " << std::showbase  << std::bitset<8>(adcs.back()) << " and original adc " << std::bitset<8>(adc) << std::dec ;
		}//if verbose

		tdc/=16; // do LE (leading edge) TDC
		LEtdcs.push_back( tdc );
		cids.push_back( (uint8_t)cid ); 
	  }// over samples 
	  if (firstCID == -1)
		firstCID=cids.back(); //just store the 5th one, doesn't matter, if all channels are aligned then cids should match at any given time sample
	  if (firstCID!=cids.back())
		isCIDunsync=true; //any one channel not aligned is enough to set this bool 

	  qieOuts.at(idx).setADC( digi.getADC() );
	  qieOuts.at(idx).setTDC( LEtdcs );
	}//over digis
	if (isCIDunsync)
	  ldmx_log(debug) << "Found unsynced CIDs!" ;
	if (isCIDskipped)
	  ldmx_log(info) << "Found skipped CIDs!" ;

    
	//data format:
	// RM ID. we don't set it for testbeam. in LDMX let's say it's 0 for tag, 1 for up, 2 for dn -- later this can be read using moduleID... 
	// 16 bit trigger ID.
	// 4 bits of flags, then 4 reserved 0 for now 
	// some 8-bit error word/checksum.
	// all channel 8-bit ADCs.
	// all channel TDCS.
	// done.
  
	uint16_t triggerID =event.getEventHeader().getEventNumber();
	uint16_t header0;
	uint8_t randomChecksum = 30;  // really, this is just a random number for now. TODO> implement a checksum
	uint8_t flags;  //we use this to contain the reserved 0's for now 
	//put it all in, at the assigned position
	flags |= (isCRC0malformed << QIEStream::CRC0_ERR_POS);
	flags |= (isCRC1malformed << QIEStream::CRC1_ERR_POS);
	flags |= (isCIDunsync<< QIEStream::CID_UNSYNC_POS);
	flags |= (isCIDskipped<< QIEStream::CID_SKIP_POS);
	ldmx_log(debug) << "FLAGS: " << std::bitset<8>(flags) ;
  
	std::vector <int> outWord;
	outWord.push_back( triggerID);
	outWord.push_back( flags );
	outWord.push_back( randomChecksum);

	if (verbose_) {
	  std::cout << "header word " ;
	  for (auto word: outWord)
		std::cout <<  std::bitset<16>(word)  << " " ;
	  std::cout  << std::endl;
	}
  
	//now write this in sequence: ADC of all channels, then TDC; repeat for all samples 
	for (int iS = 0; iS < nSamp; iS++) { 
	  for (int iQ = 0; iQ < nChannels_ ; iQ++) {
		outWord.push_back( qieOuts.at(iQ).getADC().at(iS) ) ;	  
	  }//over channels : ADC 
	  for (int iQ = 0; iQ < nChannels_ ; iQ++) {
		outWord.push_back( qieOuts.at(iQ).getTDC().at(iS)  ) ;
	  }//over channels: TDC
	}//over time samples

  
	//in verbose mode, print this all to screen 
	if (verbose_) {
	  std::cout << "total word " ;
	  int widx = 0;
	  for (auto word: outWord) { 
		if ((widx - 3)%nChannels_ == 0 ) { 
		  int sample = (widx - 3)/nChannels_ ;
		  if ( sample%2 == 0 )
			std::cout << "\n sample " << sample/2 << " |  "; 
		  else
			std::cout << "\n TDC:        " ;
		}
		std::cout <<  word  << " " ;
		//std::cout <<  std::bitset<8>(word)  << " " ;  //for binary output format
		widx++;
	  }
	  std::cout  << std::endl;
	}//if verbose

	event.add(outputCollection_, outWord);

  }
  
void QIEEncoder::onFileOpen() {
  ldmx_log(debug) << "Opening file!";


return;
}

void QIEEncoder::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void QIEEncoder::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void QIEEncoder::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}


}  // namespace trigscint                                                                                                                           

DECLARE_PRODUCER_NS(trigscint, QIEEncoder);

