#include "TrigScint/QIEDecoder.h"

#include <bitset>
#include <TMath.h>


namespace trigscint {

 void QIEDecoder::configure(
	  framework::config::Parameters &ps) {
  // Configure this instance of the encoder
   outputCollection_ = ps.getParameter<std::string>("output_collection");
   inputCollection_ = ps.getParameter<std::string>("input_collection");
   inputPassName_ = ps.getParameter<std::string>("input_pass_name");
   channelMapFileName_ = ps.getParameter<std::string>("channel_map_file");
   nChannels_ = ps.getParameter<int>("number_channels");
   nSamples_ = ps.getParameter<int>("number_time_samples");
   isRealData_ = ps.getParameter<bool>("is_real_data");
   verbose_ = ps.getParameter<bool>("verbose");

   ldmx_log(debug) << "In configure, got parameters:" <<
	 "\noutput_collection = " << outputCollection_ <<
	 "\ninput_collection = " << inputCollection_ <<
	 "\ninput_pass_name  = " << inputPassName_ <<
	 "\nchannel_map_file = " <<  channelMapFileName_ <<
	 "\nnumber_channels  = " <<  nChannels_ <<
	 "\nnumber_time_samples  = " <<  nSamples_ <<
	 "\nis_real_data  = " <<  isRealData_ <<
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
	
	//	ldmx::EventHeader *eh = (ldmx::EventHeader*)event.getEventHeaderPtr();
	//eh->setIsRealData(isRealData_); //doesn't help
	
	/*   // comment for now, requires pushing change ot EventHeader
	event.getEventHeader().setIsRealData(isRealData_);
	ldmx_log(debug) << "Decoder bool isRealData = " << isRealData_ << " and after attempt of setting it, event header returns " <<  event.getEventHeader().isRealData();
	*/
	  //turns out this need to be configurable for now, to read real data
	int nSamp = nSamples_; //QIEStream::NUM_SAMPLES ;  
	ldmx_log(debug) << "num samples = " << nSamp;
	
	ldmx_log(debug) << "Looking up input collection " << inputCollection_ << "_" << inputPassName_;
	const auto eventStream{event.getCollection<uint8_t>( inputCollection_, inputPassName_)};
	ldmx_log(debug) << "Got input collection" << inputCollection_ << "_" << inputPassName_;
	
	
  uint32_t timeEpoch=0;
  //these don't have to be in any particular order, position is anyway looked up from definition in header
  for (int iW = 0 ; iW < QIEStream::TIMESTAMP_LEN_BYTES; iW++) {
	int pos = QIEStream::TIMESTAMP_POS + iW;
    uint8_t timeWord = eventStream.at( pos );
    ldmx_log(debug) << "time stamp word at position " << pos << " (with iW = " << iW << ") = " << std::bitset<8>(timeWord ) ;
    timeEpoch |= (timeWord << iW*8); //shift by a byte at a time
  }

  uint32_t timeClock=0;
  for (int iW = 0 ; iW < QIEStream::TIMESTAMPCLOCK_LEN_BYTES; iW++) {
	int pos = QIEStream::TIMESTAMPCLOCK_POS + iW;
    uint8_t timeWord = eventStream.at( pos );
    ldmx_log(debug) << "time stamp ns word at position " << pos << " (with iW = " << iW << ") = " << std::bitset<8>(timeWord ) ;
    timeClock |= (timeWord << iW*8); //shift by a byte at a time
  }

  uint32_t timeSpill=0;
  ldmx_log(debug) << "Before starting, timeSpill = " << timeSpill << " (" << std::bitset<64>(timeSpill) << ", or, " << std::hex << timeSpill << std::dec << ") counts since start of spill";

  for (int iW = 0 ; iW < QIEStream::TIMESINCESPILL_LEN_BYTES; iW++) {
	int pos = QIEStream::TIMESINCESPILL_POS + iW;
    uint8_t timeWord = eventStream.at( pos );
    ldmx_log(debug) << "time since spill word at position " << pos << " (with iW = " << iW << ") = " << std::bitset<8>(timeWord ) ;
    timeSpill |= (timeWord << iW*8); //shift by a byte at a time
  }
  ldmx_log(debug) << "time stamp words are : " << timeEpoch << " (" << std::bitset<64>(timeEpoch) << ") and " << timeClock << " (" << std::bitset<64>(timeClock) << ") clock ticks, and " << timeSpill << " (" << std::bitset<64>(timeSpill) << ", or, " << std::hex << timeSpill << std::dec << ") counts since start of spill";

  int sigBitsSkip=6; //the first 6 bits are part of something else. 
  int divisor=TMath::Power(2, 32-sigBitsSkip);
  timeSpill=timeSpill%divisor; //remove them by taking remainder in division by the values of the last skipped bit
  //timeSpill=timeSpill/spillTimeConv_;
  ldmx_log(debug) << "After taking it mod 2^" << 32-sigBitsSkip << " (which is " << divisor << ", spill time is " << timeSpill;
  event.getEventHeader().setIntParameter("timeSinceSpill", timeSpill);
  //  event.getEventHeader().setTimeSinceSpill(timeSpill); //not working 

  TTimeStamp *timeStamp = new TTimeStamp(timeEpoch);
  //  timeStamp->SetNanoSec(timeClock); //not sure about this one...
  event.getEventHeader().setTimestamp(*timeStamp);

  // trigger ID event number
  uint32_t triggerID =0; 

  for (int iW = 0 ; iW < QIEStream::TRIGID_LEN_BYTES; iW++) {
	//assume the whole 3B are written as a single 24 bits word
	int pos = QIEStream::TRIGID_POS+iW;
	uint8_t tIDword = eventStream.at( pos );
	ldmx_log(debug) << "trigger word at position " << pos << " (with iW = " << iW << ") = " << std::bitset<8>(tIDword ) ;
	triggerID |= (tIDword << iW*8); //shift by a byte at a time
  }
  
  //  ldmx_log(debug) << " got triggerID " << std::bitset<16>(triggerID) ; //eventStream.at(0);
  ldmx_log(debug) << " got triggerID " << std::bitset<32>(triggerID) ; //eventStream.at(0);

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
  uint8_t referenceChecksum = 0;  // really, this is just empty for now. TODO> implement a checksum set/get 
  int checksum{ (flags >> QIEStream::CHECKSUM_POS) & mask8<QIEStream::CHECKSUM_SIZE_BITS>::m}; //eventStream.at(QIEStream::CRC0_ERR_POS) QIEStream::CHECKSUM_POS);
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
  int iWstart = std::max( std::max(QIEStream::ERROR_POS, QIEStream::CHECKSUM_POS),
						  QIEStream::TRIGID_POS+(QIEStream::TRIGID_LEN_BYTES)) +1;  //make sure we're at end of header
  int nWords = nSamp*nChannels_*2 + iWstart; //1 ADC, 1 TDC per channel per sample, + the words in the header
  int iWord = iWstart;
  ldmx_log(debug) << "Event parsing starts at vector idx " << iWstart << " and nWords = " << nWords; 
  // outer loop: over nSamples
  // inner loop over nChannels to get ADCs, then repeat to get TDCs
  for (int iS = 0; iS < nSamp; iS++) {                                                                                                     
	for (int iQ = 0; iQ < nChannels_ ; iQ++) {
	  if ( iWord >= nWords ) {
        ldmx_log(fatal) << "More words than expected! Breaking ADC loop in sample " << iS << " at iQ = " << iQ;
        break;
      }
	  uint8_t val = eventStream.at(iWord);
	  if (val > 0) { 	//add only the digis with non-zero ADC value
		ldmx_log(debug) << "got ADC value " << (unsigned)val << " at channel (elec) idx " << iQ ;
		  if ( ADCmap.find( iQ ) == ADCmap.end() ) { // we have a new channel 
		  std::vector <int > adcs(nSamp, 0);
		  ADCmap.insert(std::pair<int, std::vector <int> >(iQ, adcs));
		}
		ADCmap[ iQ ].at(iS) = val;
	  }
	  iWord++;
	}
	for (int iQ = 0; iQ < nChannels_ ; iQ++) {
	  if ( iWord >= nWords ) {
        ldmx_log(debug) << "More words than expected! Breaking TDC loop in sample " << iS << " at iQ = " << iQ;
		break;
	  }
	  uint8_t val = eventStream.at(iWord);
	  if (val > 0) { // TODO: check if this channel is also present in ADC map? in the end? 
		ldmx_log(debug) << "got TDC value " << (unsigned)val << " at channel (elec) idx " << iQ ;;
		if ( TDCmap.find( iQ ) == TDCmap.end() ) { // we have a new channel 
		  std::vector <int > tdcs(nSamp, 0);
		  TDCmap.insert(std::pair<int, std::vector <int> >(iQ, tdcs));
		}
		//this is LETDC; only the two most significant bits included
		// they are shipped as least significant bits --> shift them 
		TDCmap[ iQ ].at(iS) = (val+1)*16;  // want LE TDC = 3 to correspond to 64 > 49 (which is maxTDC in sim)
	  }
	  iWord++;
	}
	ldmx_log(debug) << "Done with sample " << iS ;

  }

  ldmx_log(debug) << "Done reading in header, ADC and TDC for event " << (unsigned)triggerID ;
  for (std::map<int, std::vector<int>>::iterator itr = ADCmap.begin(); itr != ADCmap.end(); ++itr) {
	TrigScintQIEDigis  digi;
	digi.setADC( itr->second );
	if (channelMap_.find( itr->first ) == channelMap_.end() ) {
	  ldmx_log(fatal) << "Couldn't find the bar ID corresponding to electronics ID " << itr->first << "!! Skipping." ;
	  continue;
	}
	int bar = channelMap_[itr->first];
	digi.setElecID( itr->first );
	digi.setChanID( bar );
	digi.setTDC( TDCmap[itr->first] );
	digi.setTimeSinceSpill(timeSpill);
	if (bar == 0)
	  ldmx_log(debug) << "for bar 0, got time since spill "<< digi.getTimeSinceSpill();
	outDigis.push_back( digi );
	ldmx_log(debug) << "Iterator points to key " << itr->first
					<< " and mapped channel supposedly  is " << bar ;
	ldmx_log(debug) << "Made digi with elecID = " << digi.getElecID()
					<< ", barID = " << digi.getChanID()
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

