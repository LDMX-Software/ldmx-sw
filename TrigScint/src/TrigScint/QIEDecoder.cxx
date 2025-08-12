#include "TrigScint/QIEDecoder.h"

#include <TMath.h>

#include <bitset>

namespace trigscint {

void QIEDecoder::configure(framework::config::Parameters &ps) {
  // Configure this instance of the encoder
  outputCollection_ = ps.getParameter<std::string>("output_collection");
  inputCollection_ = ps.getParameter<std::string>("input_collection");
  inputPassName_ = ps.getParameter<std::string>("input_pass_name");
  channelMapFileName_ = ps.getParameter<std::string>("channel_map_file");
  nChannels_ = ps.getParameter<int>("number_channels");
  nSamples_ = ps.getParameter<int>("number_time_samples");
  isRealData_ = ps.getParameter<bool>("is_real_data");
  verbose_ = ps.getParameter<bool>("verbose");

  ldmx_log(debug) << "In configure, got parameters:" << "\noutput_collection = "
                  << outputCollection_
                  << "\ninput_collection = " << inputCollection_
                  << "\ninput_pass_name  = " << inputPassName_
                  << "\nchannel_map_file = " << channelMapFileName_
                  << "\nnumber_channels  = " << nChannels_
                  << "\nnumber_time_samples  = " << nSamples_
                  << "\nis_real_data  = " << isRealData_
                  << "\nverbose          = " << verbose_;

  channelMapFile_.open(channelMapFileName_, std::ios::in);
  if (!channelMapFile_.is_open()) {
    EXCEPTION_RAISE(
        "BadMapFile",
        "The channel mapping file cannot be opened.");  // <-- appears this
                                                        // needs implementing
                                                        // first
    ldmx_log(fatal) << "The channel mapping file cannot be opened.";
    return;
  }
  int ch_id, el_id;
  while (!channelMapFile_.eof()) {
    channelMapFile_ >> el_id >> ch_id;
    // make the map based on electronics ID, to look up channel ID.
    // the reason is, we will only know the elecID from the position
    // of the word in the stream. so these need to be strictly ordered.
    // writing the right bar in the right position is easier if we can just
    // read this map from beginning to end.
    // barID can always be set, or looked up, as a property of the digi.

    // here make the elecID the key (other way around when encoding)
    channelMap_.insert(std::pair<int, int>(el_id, ch_id));
    ldmx_log(debug) << el_id << "  chID " << ch_id;
  }
  channelMapFile_.close();
  if (el_id != nChannels_ - 1)
    ldmx_log(fatal) << "The set number of channels " << nChannels_
                    << " seems not to match the number from the map (+1) :"
                    << el_id;
  return;
}

void QIEDecoder::produce(framework::Event &event) {
  ldmx_log(debug) << "QIEDecoder: produce() starts! Event number: "
                  << event.getEventHeader().getEventNumber();

  // turns out this need to be configurable for now, to read real data
  int n_samp = nSamples_;  // QIEStream::NUM_SAMPLES ;
  ldmx_log(debug) << "num samples = " << n_samp;

  ldmx_log(debug) << "Looking up input collection " << inputCollection_ << "_"
                  << inputPassName_;
  const auto event_stream{
      event.getCollection<uint8_t>(inputCollection_, inputPassName_)};
  ldmx_log(debug) << "Got input collection" << inputCollection_ << "_"
                  << inputPassName_;

  uint32_t time_epoch = 0;
  // these don't have to be in any particular order, position is anyway looked
  // up from definition in header
  for (int i_w = 0; i_w < QIEStream::TIMESTAMP_LEN_BYTES; i_w++) {
    int pos = QIEStream::TIMESTAMP_POS + i_w;
    uint8_t time_word = event_stream.at(pos);
    ldmx_log(debug) << "time stamp word at position " << pos
                    << " (with iW = " << i_w
                    << ") = " << std::bitset<8>(time_word);
    time_epoch |= (time_word << i_w * 8);  // shift by a byte at a time
  }

  uint32_t time_clock = 0;
  for (int i_w = 0; i_w < QIEStream::TIMESTAMPCLOCK_LEN_BYTES; i_w++) {
    int pos = QIEStream::TIMESTAMPCLOCK_POS + i_w;
    uint8_t time_word = event_stream.at(pos);
    ldmx_log(debug) << "time stamp ns word at position " << pos
                    << " (with iW = " << i_w
                    << ") = " << std::bitset<8>(time_word);
    time_clock |= (time_word << i_w * 8);  // shift by a byte at a time
  }

  uint32_t time_spill = 0;
  ldmx_log(debug) << "Before starting, timeSpill = " << time_spill << " ("
                  << std::bitset<64>(time_spill) << ", or, " << std::hex
                  << time_spill << std::dec << ") counts since start of spill";

  for (int i_w = 0; i_w < QIEStream::TIMESINCESPILL_LEN_BYTES; i_w++) {
    int pos = QIEStream::TIMESINCESPILL_POS + i_w;
    uint8_t time_word = event_stream.at(pos);
    ldmx_log(debug) << "time since spill word at position " << pos
                    << " (with iW = " << i_w
                    << ") = " << std::bitset<8>(time_word);
    time_spill |= (time_word << i_w * 8);  // shift by a byte at a time
  }
  ldmx_log(debug) << "time stamp words are : " << time_epoch << " ("
                  << std::bitset<64>(time_epoch) << ") and " << time_clock << " ("
                  << std::bitset<64>(time_clock) << ") clock ticks, and "
                  << time_spill << " (" << std::bitset<64>(time_spill) << ", or, "
                  << std::hex << time_spill << std::dec
                  << ") counts since start of spill";

  int sig_bits_skip = 6;  // the first 6 bits are part of something else.
  int divisor = TMath::Power(2, 32 - sig_bits_skip);
  // remove them by taking remainder in division by
  // the values of the last skipped bit
  time_spill = time_spill % divisor;
  ldmx_log(debug) << "After taking it mod 2^" << 32 - sig_bits_skip
                  << " (which is " << divisor << ", spill time is "
                  << time_spill;
  event.getEventHeader().setIntParameter("timeSinceSpill", time_spill);

  TTimeStamp *time_stamp = new TTimeStamp(time_epoch);
  event.getEventHeader().setTimestamp(*time_stamp);

  // trigger ID event number
  uint32_t trigger_id = 0;

  for (int i_w = 0; i_w < QIEStream::TRIGID_LEN_BYTES; i_w++) {
    // assume the whole 3B are written as a single 24 bits word
    int pos = QIEStream::TRIGID_POS + i_w;
    uint8_t t_i_dword = event_stream.at(pos);
    ldmx_log(debug) << "trigger word at position " << pos
                    << " (with iW = " << i_w
                    << ") = " << std::bitset<8>(t_i_dword);
    trigger_id |= (t_i_dword << i_w * 8);  // shift by a byte at a time
  }

  //  ldmx_log(debug) << " got triggerID " << std::bitset<16>(triggerID) ;
  ldmx_log(debug) << " got triggerID " << std::bitset<32>(trigger_id);

  if (trigger_id != event.getEventHeader().getEventNumber()) {
    // this probably only applies to digi emulation,
    // unless an event number is explicitly set in unpacking
    ldmx_log(fatal) << "Got event number mismatch: framework reports "
                    << event.getEventHeader().getEventNumber()
                    << ", stream says " << trigger_id;
  }

  // error word
  /* the error word contains
         - 4 trailing reserved 0's, for now
         - isCIDunsync : if there is a mismatch between CID reported by channels
     within the same time sample
         - isCIDskipped : if there is a gap in the CID increment of a channel
     beweeen samples
         - isCRC0malformed : if there was an issue with CRC from fiber0
         - isCRC1malformed : if there was an issue with CRC from fiber1
  */
  uint8_t flags = event_stream.at(QIEStream::ERROR_POS);

  bool is_ci_dskipped{static_cast<bool>((flags >> QIEStream::CID_SKIP_POS) &
                                      mask8<QIEStream::FLAG_SIZE_BITS>::m)};
  bool is_ci_dunsync{static_cast<bool>((flags >> QIEStream::CID_UNSYNC_POS) &
                                     mask8<QIEStream::FLAG_SIZE_BITS>::m)};
  // These are unused, should they be? FIXME
  // bool isCRC1malformed{static_cast<bool>((flags >> QIEStream::CRC1_ERR_POS) &
  //                                        mask8<QIEStream::FLAG_SIZE_BITS>::m)};
  // bool isCRC0malformed{static_cast<bool>((flags >> QIEStream::CRC0_ERR_POS) &
  //                                       mask8<QIEStream::FLAG_SIZE_BITS>::m)};

  // checksum
  // really, this is just empty for now.
  // TODO: implement a checksum set/get
  uint8_t reference_checksum = 0;
  int checksum{(flags >> QIEStream::CHECKSUM_POS) &
               mask8<QIEStream::CHECKSUM_SIZE_BITS>::
                   m};  // eventStream.at(QIEStream::CRC0_ERR_POS)
                        // QIEStream::CHECKSUM_POS);
  if (checksum != reference_checksum)
    ldmx_log(fatal) << "Got checksum mismatch: expected "
                    << (int)reference_checksum << ", stream says " << checksum;
  if (is_ci_dunsync) ldmx_log(debug) << "Found unsynced CIDs!";
  if (is_ci_dskipped) ldmx_log(fatal) << "Found skipped CIDs!";

  /* -- TS event header done; read the channel contents -- */
  std::vector<trigscint::TrigScintQIEDigis> out_digis;
  std::map<int, std::vector<int>> ad_cmap;
  std::map<int, std::vector<int>> td_cmap;

  // read in words from the stream. line them up per channel and time sample.
  // channels are in the electronics ordering
  int i_wstart =
      std::max(std::max(QIEStream::ERROR_POS, QIEStream::CHECKSUM_POS),
               QIEStream::TRIGID_POS + (QIEStream::TRIGID_LEN_BYTES)) +
      1;  // make sure we're at end of header
  int n_words =
      n_samp * nChannels_ * 2 + i_wstart;  // 1 ADC, 1 TDC per channel per sample,
                                         // + the words in the header
  int i_word = i_wstart;
  ldmx_log(debug) << "Event parsing starts at vector idx " << i_wstart
                  << " and nWords = " << n_words;
  // outer loop: over nSamples
  // inner loop over nChannels to get ADCs, then repeat to get TDCs
  for (int i_s = 0; i_s < n_samp; i_s++) {
    for (int i_q = 0; i_q < nChannels_; i_q++) {
      if (i_word >= n_words) {
        ldmx_log(fatal)
            << "More words than expected! Breaking ADC loop in sample " << i_s
            << " at iQ = " << i_q;
        break;
      }
      uint8_t val = event_stream.at(i_word);
      if (val > 0) {  // add only the digis with non-zero ADC value
        ldmx_log(debug) << "got ADC value " << (unsigned)val
                        << " at channel (elec) idx " << i_q;
        if (ad_cmap.find(i_q) == ad_cmap.end()) {  // we have a new channel
          std::vector<int> adcs(n_samp, 0);
          ad_cmap.insert(std::pair<int, std::vector<int>>(i_q, adcs));
        }
        ad_cmap[i_q].at(i_s) = val;
      }
      i_word++;
    }
    for (int i_q = 0; i_q < nChannels_; i_q++) {
      if (i_word >= n_words) {
        ldmx_log(debug)
            << "More words than expected! Breaking TDC loop in sample " << i_s
            << " at iQ = " << i_q;
        break;
      }
      uint8_t val = event_stream.at(i_word);
      if (val > 0) {  // TODO: check if this channel is also present in ADC map?
                      // in the end?
        ldmx_log(debug) << "got TDC value " << (unsigned)val
                        << " at channel (elec) idx " << i_q;
        ;
        if (td_cmap.find(i_q) == td_cmap.end()) {  // we have a new channel
          std::vector<int> tdcs(n_samp, 0);
          td_cmap.insert(std::pair<int, std::vector<int>>(i_q, tdcs));
        }
        // this is LETDC; only the two most significant bits included
        // they are shipped as least significant bits --> shift them
        // TDCmap[iQ].at(iS) = (val + 1) * 16;  // want LE TDC = 3 to correspond
        // to
        // 64 > 49 (which is maxTDC in sim)
        td_cmap[i_q].at(i_s) = val;  // have full 6-bit TDC at SLAC
      }
      i_word++;
    }
    ldmx_log(debug) << "Done with sample " << i_s;
  }

  ldmx_log(debug) << "Done reading in header, ADC and TDC for event "
                  << trigger_id;
  for (std::map<int, std::vector<int>>::iterator itr = ad_cmap.begin();
       itr != ad_cmap.end(); ++itr) {
    TrigScintQIEDigis digi;
    digi.setADC(itr->second);
    if (channelMap_.find(itr->first) == channelMap_.end()) {
      ldmx_log(fatal)
          << "Couldn't find the bar ID corresponding to electronics ID "
          << itr->first << "!! Skipping.";
      continue;
    }
    int bar = channelMap_[itr->first];
    digi.setElecID(itr->first);
    digi.setChanID(bar);
    digi.setTDC(td_cmap[itr->first]);
    digi.setTimeSinceSpill(time_spill);
    if (bar == 0)
      ldmx_log(debug) << "for bar 0, got time since spill "
                      << digi.getTimeSinceSpill();
    out_digis.push_back(digi);
    ldmx_log(debug) << "Iterator points to key " << itr->first
                    << " and mapped channel supposedly  is " << bar;
    ldmx_log(debug) << "Made digi with elecID = " << digi.getElecID()
                    << ", barID = " << digi.getChanID() << ", third adc value "
                    << digi.getADC().at(2) << " and third tdc "
                    << digi.getTDC().at(2);
  }

  event.add(outputCollection_, out_digis);
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

DECLARE_PRODUCER(trigscint::QIEDecoder);
