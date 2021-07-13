
#include "Packing/Translators/Hgcroc.h"

#include <bitset>
#include <iomanip>

namespace packing {
namespace translators {

/**
 * @class BufferReader
 * Read the buffer type, only keeping
 * the lowest 32bits of the 64 bit words.
 */
class BufferReader {
 public:
  BufferReader(const BufferType& b) : buffer_{b}, i_read_{0} {}
  const uint32_t& now() {
    return reinterpret_cast<const uint32_t*>(&buffer_.at(i_read_))[0];
  }
  bool next(bool should_exist = true) {
    i_read_++;
    if (i_read_ == buffer_.size()) {
      if (should_exist)
        throw std::out_of_range("next word should exist");
      else
        return false;
    }
    std::cout << std::bitset<32>(now()) << std::endl;
    return true; 
  }
 private:
  // current buffer we are reading
  const BufferType& buffer_;
  // current index in buffer we are reading
  std::size_t i_read_;
};

Hgcroc::Hgcroc(const framework::config::Parameters& ps) : Translator(ps) {}

bool Hgcroc::canTranslate(const std::string& name) const {
  static const std::string sub_str_match{"PrecisionHgcroc"};
  return (name.find(sub_str_match) != std::string::npos);
}

void Hgcroc::decode(framework::Event& event, const BufferType& buffer) {
  /** Re-sort the data from grouped by bunch to by channel
   * The readout chip streams the data off of it, so it doesn't
   * have time to re-group the signals across multiple bunches (samples)
   * by their channel ID. We need to do that here.
   */
  // fill map of **electronic** IDs to the digis that were read out
  std::map<uint32_t, std::vector<ldmx::HgcrocDigiCollection::Sample>> data;
  BufferReader r{buffer};
  do {
    try {
      /** Decode Bunch Header
       * We have a few words of header material before the actual data.
       * This header material is assumed to be encoded as in Table 3
       * of the DAQ specs.
       *
       * <name> (bits)
       *
       * VERSION (4) | FPGA_ID (8) | NLINKS (6) | 0 | LEN (12)
       * BX ID (12) | RREQ (10) | OR (10)
       * RID ok (1) | CDC ok (1) | LEN3 (6) |
       *  RID ok (1) | CDC ok (1) | LEN2 (6) |
       *  RID ok (1) | CDC ok (1) | LEN1 (6) |
       *  RID ok (1) | CDC ok (1) | LEN0 (6)
       * ... other listing of links ...
       */
      uint32_t version{(r.now() >> 12 + 1 + 6 + 8) & mask<4>::m};
      std::cout << std::bitset<32>(r.now())  << " -> version " << version << std::endl;
      uint32_t one{1};
      if (version != one)
        EXCEPTION_RAISE("VersMis", "Hgcroc Translator only knows version 1.");
    
      uint32_t fpga{(r.now() >> 12 + 1 + 6) & mask<8>::m};
      uint32_t nlinks{(r.now() >> 12 + 1) & mask<6>::m};
      uint32_t len{r.now() & mask<12>::m};
    
      std::cout << "fpga: " << fpga << ", nlinks: " << nlinks << ", len: " << len << std::endl;
      r.next();
    
      uint32_t bx_id{(r.now() >> 10 + 10) & mask<12>::m};
      uint32_t rreq{(r.now() >> 10) & mask<10>::m};
      uint32_t orbit{r.now() & mask<10>::m};
    
      std::cout << "bx_id: " << bx_id << ", rreq: " << rreq << ", orbit: " << orbit << std::endl;
      std::vector<uint32_t> num_channels_per_link(nlinks,0);
      for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
        if (i_link % 4 == 0) r.next();
        uint32_t shift_in_word{8 * i_link % 4};
        bool rid_ok{(r.now() >> shift_in_word + 7) & mask<1>::m == 1};
        bool cdc_ok{(r.now() >> shift_in_word + 6) & mask<1>::m == 1};
        num_channels_per_link[i_link] = (r.now() >> shift_in_word) & mask<6>::m;
        std::cout << num_channels_per_link.at(i_link) << std::endl;
      }
    
      /** Decode Each Link in Sequence
       * Now we should be decoding each link serially
       * where each link was encoded as in Table 4 of
       * the DAQ specs
       *
       * ROC_ID (16) | CRC ok (1) | 00000 | RO Map (8)
       * RO Map (32)
       */
    
      for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
        // move on from last word counting links or previous link
        std::cout << "RO Link " << i_link << std::endl;
        r.next();
        uint32_t roc_id{(r.now() >> 8 + 5 + 1) & mask<16>::m};
        bool crc_ok{(r.now() >> 8 + 5) & mask<1>::m == 1};
        uint32_t ro_map_39_32{r.now() & mask<8>::m};
        r.next();
        uint32_t ro_map_31_0{r.now() & mask<32>::m};
    
        std::cout << "Start looping through channels..." << std::endl;
        // loop through channels on this link, 
        //  check if they have been readout before saving word
        for (uint32_t i_chan{0}; i_chan < 40; i_chan++) {
          bool has_been_read{false};
          if (i_chan < 32) {
            has_been_read = ((ro_map_31_0 >> i_chan & mask<1>::m) == 1);
          } else {
            has_been_read = ((ro_map_39_32 >> (i_chan - 31) & mask<1>::m) == 1);
          }
    
          // skip zero-suppressed channels
          if (not has_been_read) continue;
    
          // next word is this channel
          r.next();
    
          if (i_chan == 0) { 
            /** Special "Header" Word from ROC
             * 0101 | BXID (12) | RREQ (6) | OR (3) | HE (3) | 0101
             */
          } else if (i_chan == 1) {
            /** Common Mode Channels
             * 10 | 0000000000 | Common Mode ADC 0 (10) | Common Mode ADC 1 (10)
             */
          } else if (i_chan == 39) {
            /** CRC Checksum form ROC
             */
            uint32_t roc_crc{r.now() & mask<32>::m};
          } else {
            /// DAQ Channels
      
            /** Generate Packed Electronics ID
             * Link Index i_link
             * Channel Index i_chan
             * ROC ID roc_id
             * FPGA ID fpga
             * are all available.
             * For now, we just generate a dummy mapping
             * using the link and channel indices.
             */
            uint32_t eid{i_link * 100 + i_chan};
      
            // copy data into EID->sample map
            data[eid].emplace_back(r.now() & mask<32>::m);
          }  // type of channel
        }    // loop over channels (j in Table 4)
        std::cout << "done looping through channels" << std::endl;

        // next word is CDC checksum
        r.next();
        uint32_t cdc{r.now() & mask<32>::m};
      }      // loop over links
    
      // next word is CDC checksum
      r.next();
      uint32_t cdc{r.now() & mask<32>::m};
    } catch (std::out_of_range&) {
      EXCEPTION_RAISE("MisFormat",
          "Recieved raw data that was not formatted correctly.");
    }
  } while (r.next(false)); 

  /** Translation
   * The actual translation done here is the translation from electronic IDs
   * to detector IDs. The unpacking of the 32-bit word samples for each channel
   * is done by the HgcrocDigiCollection::Sample class and is done on-the-fly
   * in order to save disk-space.
   */
  ldmx::HgcrocDigiCollection unpacked_data;
  // TODO: Can we assume that all the channels have the same number of bunches?
  unpacked_data.setNumSamplesPerDigi(data.begin()->second.size());
  // Electronic ID <-> Detector ID while copying into collection data structure
  for (auto const& [eid, digi] : data) {
    // TODO: Insert EID --> DetID mapping here
    auto channel = eid;
    unpacked_data.addDigi(channel, digi);
  }

  event.add("EcalDigis", unpacked_data);
}

}  // namespace translators
}  // namespace packing

DECLARE_PACKING_TRANSLATOR(packing::translators, Hgcroc)
