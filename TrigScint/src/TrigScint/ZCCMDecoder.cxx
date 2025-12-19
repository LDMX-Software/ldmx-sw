#include "TrigScint/ZCCMDecoder.h"

#include <TMath.h>

#include <bitset>

namespace trigscint {

void ZCCMDecoder::configure(framework::config::Parameters &ps) {
  // Configure this instance of the encoder
  output_collection_ = ps.get<std::string>("output_collection");
  input_collection_ = ps.get<std::string>("input_collection");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  module_map_file_name_ = ps.get<std::string>("module_map_file");
  channel_map_file_name_ = ps.get<std::string>("channel_map_file");
  n_channels_ = ps.get<int>("number_channels");
  n_samples_ = ps.get<int>("number_time_samples");
  is_real_data_ = ps.get<bool>("is_real_data");
  //  daq_junk_words_len_bytes_ = ps.get<int>("daq_extra_header_bytes");
  //daq_junk_tail_len_bits_ = ps.get<int>("daq_trailing_header_bits");

  ldmx_log(debug) << "In configure, got parameters:" << "\noutput_collection = "
                  << output_collection_
                  << "\ninput_collection = " << input_collection_
                  << "\ninput_pass_name  = " << input_pass_name_
                  << "\nmodule_map_file = " << module_map_file_name_
                  << "\nchannel_map_file = " << channel_map_file_name_
                  << "\nnumber_channels  = " << n_channels_
                  << "\nnumber_time_samples  = " << n_samples_
                  << "\nis_real_data  = " << is_real_data_;

  channel_map_file_.open(channel_map_file_name_, std::ios::in);
  if (!channel_map_file_.is_open()) {
    EXCEPTION_RAISE(
        "BadMapFile",
        "The channel mapping file cannot be opened.");  
    return;
  }
  int n_chan_from_map = -1; //we will in effect count newlines
  int ch_id, el_id;
  while (!channel_map_file_.eof()) {
    channel_map_file_ >> el_id >> ch_id;
    // make the map based on electronics ID, to look up channel ID.
    // the reason is, the elecID is given by the position of the word in
    // the output. so these need to be strictly ordered.
    // writing the right bar in the right position is easier if we can just
    // read this map from beginning to end.
    // barID can always be set, or looked up, as a property of the digi.

    // here make the elecID the key (other way around when encoding)
    channel_map_.insert(std::pair<int, int>(el_id, ch_id));
    ldmx_log(debug) << el_id << "  chID " << ch_id;
    n_chan_from_map++; //will increment on last newline as well
  }
  channel_map_file_.close();
  if (n_chan_from_map != n_channels_ )
    ldmx_log(fatal) << "The set number of channels: " << n_channels_
                    << " does not match the number obtained from the map :"
                    << n_chan_from_map;

  //and similarly for the lane-->module mapping

    module_map_file_.open(module_map_file_name_, std::ios::in);
  if (!module_map_file_.is_open()) {
    ldmx_log(fatal) << "The module mapping file cannot be opened.";
    return;
  }
  int module_id, lane_id;
  while (!module_map_file_.eof()) {
    module_map_file_ >> lane_id >> module_id;
    // make the map based on lane ID, to look up module ID.
    // here make the lane the key (other way around when encoding)
    module_map_.insert(std::pair<int, int>(lane_id, module_id));
    ldmx_log(debug) << "lane " << lane_id << "  --> module ID " << module_id;
    modules_used_[module_id] = 1;
  }
  module_map_file_.close();
  int n_lanes=n_channels_/ZCCMOutput::NUM_CHAN_PER_LANE;
  if (lane_id != n_lanes - 1)
    ldmx_log(fatal) << "The set number of lanes " << n_lanes
                    << " seems not to match the number obtained from the map :"
                    << lane_id + 1;

  return;
}

void ZCCMDecoder::produce(framework::Event &event) {
  ldmx_log(debug) << "ZCCMDecoder: produce() starts! Event number: "
                  << event.getEventHeader().getEventNumber();

  // this has to be configurable for now, to read real data
  int n_samp = n_samples_;  
  ldmx_log(debug) << "num samples = " << n_samp;

  ldmx_log(debug) << "Looking up input collection " << input_collection_ << "_"
                  << input_pass_name_;
  const auto event_output{
      event.getCollection<uint8_t>(input_collection_, input_pass_name_)};
  ldmx_log(debug) << "Got input collection " << input_collection_ << "_"
                  << input_pass_name_;

  /* -- PROCESS THE EVENT INFORMATION -- */
  
  uint32_t time_stamp = 0;
  for (int i_w = 0; i_w < ZCCMOutput::TIMESTAMP_LEN_BYTES; i_w++) {
    int pos = ZCCMOutput::TIMESTAMP_POS + i_w;
    uint8_t time_word = event_output.at(pos);
    ldmx_log(debug) << "time stamp word at position " << pos
                    << " (with iW = " << i_w
                    << ") = " << std::bitset<8>(time_word);
    time_stamp |= (time_word << i_w * 8);  // shift by a byte at a time
  }
   // TODO: make actual use of the time stamp in the event header
  /* the below is what was done before and does not apply for now,
     what one could do is maybe compare to the previous
     timestamp to make sure they are always incremented.
      
     //  ldmx_log(debug) << " got triggerID " << std::bitset<16>(triggerID) ;
     ldmx_log(debug) << " got triggerID " << std::bitset<32>(trigger_id);
     
     if (trigger_id != event.getEventHeader().getEventNumber()) {
     // this probably only applies to digi emulation,
     // unless an event number is explicitly set in unpacking
     ldmx_log(fatal) << "Got event number mismatch: framework reports "
     << event.getEventHeader().getEventNumber()
     << ", output says " << trigger_id;
     }
  */
  event.getEventHeader().setIntParameter("time_stamp_", time_stamp);
  TTimeStamp *ttime_stamp = new TTimeStamp(time_stamp);
  event.getEventHeader().setTimestamp(*ttime_stamp); // still not sure this works
    
  /* -- TS event header done; read the channel contents -- */

  /*
    in a loop over samples, loop over nlanes
    get ADC, TDC, flags and lane nb
    lane number uses a full byte
    the flags word contains
         - capID (2b)
         - CE (1b, some channel alignment error flag)
         - BC0 (most of the time 0, set to 1 with fixed frequency. if channels are aligned,
	 then BC0=1 should occur in the same time sample for all of them)
         - 4 trailing reserved 0's
  */

  std::map<int, std::vector<int>> adc_map;
  std::map<int, std::vector<int>> tdc_map;
  std::map<int, std::vector<int>> cid_map;
  std::map<int, std::vector<int>> bc0_map;
  std::map<int, std::vector<int>> ce_map;
  // we need one output digi collection per pad
  std::vector< std::vector<trigscint::TrigScintQIEDigis>> out_digis;
  std::size_t n_modules = std::ranges::count_if(modules_used_,
                                 [](int x){ return x != 0; });
  for (uint i = 0; i < n_modules; i++)
    out_digis.emplace_back(std::vector<trigscint::TrigScintQIEDigis>(NULL));
  
  // read in words from the output. line them up per channel and time sample.
  // channels are in the electronics ordering
  int word_length = ZCCMOutput::SAMPLE_WORD_LEN_BYTES;
  int num_messages = ((int)event_output.size()- ZCCMOutput::EVENTDATA_POS)/word_length;

  // with one message per time sample per lane, we expect nSample*nLanes messages
  int n_lanes=n_channels_/ZCCMOutput::NUM_CHAN_PER_LANE;
  int num_expected_messages = n_samp * n_lanes;
  if (num_messages != num_expected_messages )
    ldmx_log(warn) << "Unexpected stream length! Num messages is " << num_messages
		      << ", expect " << num_expected_messages ;
  
  // read in a lane stream at a time with a while loop
  // operate on bytes, but i_word increments for each meassage i.e. each lane 
  int i_word=0;
  int start_lane = -1;
  int sample_nb = -1;
  
  while (i_word < num_messages )
    { //update the position to read the word from 
      uint word_pos = ZCCMOutput::EVENTDATA_POS + i_word*word_length;

      uint16_t empty = event_output.at(word_pos + ZCCMOutput::EMPTY_WORD_SAMPLE_WORD_POS);
      uint8_t lane = event_output.at(word_pos + ZCCMOutput::LANE_SAMPLE_WORD_POS);
      uint8_t flag = event_output.at(word_pos + ZCCMOutput::FLAGS_SAMPLE_WORD_POS);

      ldmx_log(debug) << "Start of message " << i_word
		      << ".\n\tEmpty word is " << std::bitset<16>(empty)
		      << ", read at position " << word_pos + ZCCMOutput::EMPTY_WORD_SAMPLE_WORD_POS
		      << ". \n\tFlag word is " << std::bitset<8>(flag)
		      << ", read at position " <<word_pos + ZCCMOutput::FLAGS_SAMPLE_WORD_POS 
		      << ". \n\tLane is " << std::bitset<8>(lane)
		      << ", read at position " << word_pos + ZCCMOutput::LANE_SAMPLE_WORD_POS <<  ".";
      
      if (start_lane == -1) { //it is unset
	start_lane = lane; //store the first one
	ldmx_log(debug) << "Set time sample start lane to " << start_lane;
      }
      // extract the flag info 
      int cid{static_cast<int>((flag >> ZCCMOutput::CAPID_POS_IN_FLAG) & Mask8<ZCCMOutput::CAPID_LEN_BITS>::M)};
      ldmx_log(trace) << "Got Cap ID " << cid;
      bool bc0 {static_cast<bool>((flag >> ZCCMOutput::BC0_POS_IN_FLAG) & Mask8<ZCCMOutput::BC0_LEN_BITS>::M)};
      ldmx_log(trace) << "Got BC0 flag " << bc0;
      bool ce {static_cast<bool>((flag >> ZCCMOutput::CE_POS_IN_FLAG) & Mask8<ZCCMOutput::CE_LEN_BITS>::M)};
      ldmx_log(trace) << "Got CE flag " << ce;
      int empty_bits{static_cast<int>((flag >> ZCCMOutput::EMPTY_FLAG_WORD_POS_IN_FLAG) & Mask8<ZCCMOutput::EMPTY_FLAG_WORD_LEN_BITS>::M)};
      if (empty_bits)
	ldmx_log(fatal) << "Empty bits of flag not empty: " << empty_bits;

      // TODO: check that BC0 bit is aligned across all lanes 
      // TODO: keep track of the CID cycling and flag when a CID is skipped 
      //bool is_cid_unsync{static_cast<bool>((flags >> ZCCMOutput::CID_UNSYNC_POS) &
      //                                       Mask8<ZCCMOutput::FLAG_SIZE_BITS>::M)};

      //how do we know we're at the next time sample?
      // 1. i_word has incremented by the number of lanes
      //    // i.e i_word % nLanes = 0
      // 2. we're seeing the first lane again
      // it is good, then, to check that these agree.

      if (lane == start_lane ) //we have cycled (or just started)
      	sample_nb++;

      uint8_t cid_val=cid;

      //get channel-by-channel info (ADC and TDC for this time sample)
      for (int i_c = 0; i_c < ZCCMOutput::NUM_CHAN_PER_LANE; i_c++) {
	if (i_word >= num_expected_messages) {
	  ldmx_log(fatal)
            << "More words than expected! Breaking event data loop in sample " << sample_nb
            << " at lane = " << lane << ", channel nb " << i_c;
	  break;
	}
	uint8_t adc_val = event_output.at(word_pos + ZCCMOutput::ADC_SAMPLE_WORD_POS+i_c);
	uint8_t tdc_val = event_output.at(word_pos + ZCCMOutput::TDC_SAMPLE_WORD_POS+i_c);
	// define a unique elecID for each channel in the readout, comprising of lane, module, channelNB
	// then use that to look up bar ID in the the maps
	int module = module_map_[lane]; 
	int elec_id = 100*lane + 10*module + i_c;  // numbering scheme: LLMC
	
	ldmx_log(trace) << "got ADC value " << (unsigned)adc_val
			<< " and TDC value " << (unsigned)tdc_val
			<< " at channel idx " << i_c
			<< " with elec id " << elec_id;
	if (adc_map.find(elec_id) == adc_map.end()) {  // we have a new channel
	  std::vector<int> adcs(n_samp, 0);
	  adc_map.insert(std::pair<int, std::vector<int>>(elec_id, adcs));
	}
	adc_map[elec_id].at(sample_nb) = adc_val;
	//perhaps overly prudent to check for TDC and CID as well, might tag them along on ADC
	if (tdc_map.find(elec_id) == tdc_map.end()) {  // we have a new channel
	  std::vector<int> tdcs(n_samp, 0);
	  tdc_map.insert(std::pair<int, std::vector<int>>(elec_id, tdcs));
	}
	tdc_map[elec_id].at(sample_nb) = tdc_val;
	if (cid_map.find(elec_id) == cid_map.end()) {  // we have a new channel
	  std::vector<int> cids(n_samp, 0);
	  cid_map.insert(std::pair<int, std::vector<int>>(elec_id, cids));
	  std::vector<int> bc0s(n_samp, 0);
	  bc0_map.insert(std::pair<int, std::vector<int>>(elec_id, bc0s));
	  std::vector<int> ces(n_samp, 0);
	  ce_map.insert(std::pair<int, std::vector<int>>(elec_id, ces));
	}
	cid_map[elec_id].at(sample_nb) = cid_val;
	bc0_map[elec_id].at(sample_nb) = bc0_val;
	ce_map[elec_id].at(sample_nb) = ce_val;
      }// over channels
      ldmx_log(debug) << "Done with lane " <<  int(lane)
		      << " which we think is " << i_word%n_lanes
		      << " in sample " << sample_nb;

      if (int(lane) !=  i_word%n_lanes)
	ldmx_log(fatal) << "Lane ordering has been messed up! Expect lane "
			<< i_word%n_lanes << ", but we got lane " << int(lane);
      //shift to next word
      i_word++;
    } //while event data words left in the stream

    ldmx_log(debug) << "Done reading in header, ADC and TDC for event "
    << event.getEventNumber();

  //Reading step done.
 
  /* -- make digis from all the info stored in maps -- */
  for (std::map<int, std::vector<int>>::iterator itr = adc_map.begin();
       itr != adc_map.end(); ++itr) {
    TrigScintQIEDigis digi;
    digi.setADC(itr->second);
    if (channel_map_.find(itr->first) == channel_map_.end()) {
      ldmx_log(fatal)
          << "Couldn't find the bar ID corresponding to electronics ID "
          << itr->first << "!! Skipping.";
      continue;
    }
    int bar = channel_map_[itr->first];
    digi.setElecID(itr->first);
    digi.setChanID(bar);
    int module = itr->first/10%10;
    digi.setModuleID(module);
    int lane = itr->first/100;
    digi.setLaneID(lane);
    digi.setTDC(tdc_map[itr->first]);
    digi.setCID(cid_map[itr->first]);
    digi.setBC0(bc0_map[itr->first]);
    digi.setCE(ce_map[itr->first]);
    if (bar == 0)
      ldmx_log(debug) << "for bar 0, got time since spill "
                      << digi.getTimeSinceSpill();
    out_digis[module].push_back(digi);
    ldmx_log(debug) << "Iterator points to key " << itr->first
                    << " and mapped channel supposedly  is " << bar;
    ldmx_log(debug) << "Made digi with elecID = " << digi.getElecID()
                    << ", barID = " << digi.getChanID() << ", third adc value "
                    << digi.getADC().at(2) << " and third tdc "
                    << digi.getTDC().at(2);
  }

  // name them as ordered 
  for (uint i = 0; i < n_modules; i++)
    event.add(output_collection_+Form("%i",i+1), out_digis[i]);
}

void ZCCMDecoder::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void ZCCMDecoder::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::ZCCMDecoder);
