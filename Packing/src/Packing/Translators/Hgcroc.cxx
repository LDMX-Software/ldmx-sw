
#include "Packing/Translators/Hgcroc.h"

#include <bitset>
#include <iomanip>
#include <boost/crc.hpp>

namespace packing {
namespace translators {

/**
 * @struct mask
 *
 * Lots of translators need masks of various numbers of bits. 
 * This struct allows the coder to quickly get a mask for the
 * lowest N bits.
 * 
 * Using this technique, the mask is deduced at compile time, 
 * making the code a lot more readable but no less fast!
 *
 * Example:
 *  To get the lowest 5 bits of integer i
 *    i & mask<5>::m;
 *
 * @tparam[in] N number of lowest order bits to mask for
 */
template <uint32_t N>
struct mask {
  static const uint32_t one{1};
  static const uint32_t m = (one << N) - one;
};

/**
 * @struct CRC
 *
 * The HGC ROC and FPGA use a CRC checksum to double check that the
 * data transfer has been done correctly. Boost has a CRC checksum
 * library that we can use to do this checking here as well.
 *
 * Idea for this helper struct was found on StackOverflow
 * https://stackoverflow.com/a/63237679
 * I've actually simplified it by limiting its use-case to our
 * type of data.
 */
struct CRC {
  // the object from Boost doing the summing
  boost::crc_32_type crc;
  // add a word to the sum
  void operator()(const uint32_t& w) {
    crc.process_bytes(&w, sizeof(w));
  }
  // get the checksum
  auto get() { return crc.checksum(); }
};

/**
 * @class BufferReader
 * Read the buffer type, only keeping the lowest 32bits of the 64 bit words.
 *
 * This is a very simple class, it just helps us make sure we don't
 * go past the end of the buffer while inside our oddly written loop.
 *
 * It also handles the casting for us which makes the decoding code
 * slightly simpler.
 */
class BufferReader {
 public:
  /**
   * Initialize a reader by wrapping a buffer to read.
   */
  BufferReader(const BufferType& b) : buffer_{b}, i_read_{0} {}
  /**
   * Get lowest 32-bits of current word in buffer.
   * @return uint32_t current word
   */
  const uint32_t& now() {
    return reinterpret_cast<const uint32_t*>(&buffer_.at(i_read_))[0];
  }
  /**
   * Go to next word in buffer.
   *
   * @throws std::out_of_range if should_exist is true and we reach
   * the end of the buffer
   * @param[in] should_exist set to false if we are okay with the next word
   * not existing
   * @return true if we have another word, false if we've reached the end
   */
  bool next(bool should_exist = true) {
    i_read_++;
    if (i_read_ == buffer_.size()) {
      if (should_exist)
        throw std::out_of_range("next word should exist");
      else
        return false;
    }
    return true;
  }

 private:
  // current buffer we are reading
  const BufferType& buffer_;
  // current index in buffer we are reading
  std::size_t i_read_;
};

Hgcroc::Hgcroc(const framework::config::Parameters& ps) : Translator(ps) {
  roc_version_ = ps.getParameter<int>("roc_version");
}

bool Hgcroc::canTranslate(const std::string& name) const {
  static const std::string sub_str_match{"PrecisionHgcroc"};
  return (name.find(sub_str_match) != std::string::npos);
}

void Hgcroc::decode(framework::Event& event, const BufferType& buffer) {
  /**
   * Static parameters depending on ROC version
   */
  static const unsigned int common_mode_channel{roc_version_ == 2 ? 19 : 1};

  /** Re-sort the data from grouped by bunch to by channel
   * The readout chip streams the data off of it, so it doesn't
   * have time to re-group the signals across multiple bunches (samples)
   * by their channel ID. We need to do that here.
   */
  // fill map of **electronic** IDs to the digis that were read out
  std::map<uint32_t, std::vector<ldmx::HgcrocDigiCollection::Sample>> data;
  BufferReader r{buffer};
  std::cout << buffer.size() << std::endl;
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
      CRC fpga_crc;
      fpga_crc(r.now());
      std::cout << std::bitset<32>(r.now()) << " : ";
      uint32_t version{(r.now() >> 12 + 1 + 6 + 8) & mask<4>::m};
      std::cout << "version " << version << std::flush;
      uint32_t one{1};
      if (version != one)
        EXCEPTION_RAISE("VersMis", "Hgcroc Translator only knows version 1.");

      uint32_t fpga{(r.now() >> 12 + 1 + 6) & mask<8>::m};
      uint32_t nlinks{(r.now() >> 12 + 1) & mask<6>::m};
      uint32_t len{r.now() & mask<12>::m};

      std::cout << ", fpga: " << fpga << ", nlinks: " << nlinks
                << ", len: " << len << std::endl;
      r.next();
      fpga_crc(r.now());
      std::cout << std::bitset<32>(r.now()) << " : ";

      uint32_t bx_id{(r.now() >> 10 + 10) & mask<12>::m};
      uint32_t rreq{(r.now() >> 10) & mask<10>::m};
      uint32_t orbit{r.now() & mask<10>::m};

      std::cout << "bx_id: " << bx_id << ", rreq: " << rreq
                << ", orbit: " << orbit << std::endl;
      std::vector<uint32_t> num_channels_per_link(nlinks, 0);
      for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
        if (i_link % 4 == 0) {
          r.next();
          fpga_crc(r.now());
        }
        uint32_t shift_in_word{8 * i_link % 4};
        bool rid_ok{(r.now() >> shift_in_word + 7) & mask<1>::m == 1};
        bool cdc_ok{(r.now() >> shift_in_word + 6) & mask<1>::m == 1};
        num_channels_per_link[i_link] = (r.now() >> shift_in_word) & mask<6>::m;
        std::cout << "Link " << i_link << " readout "
                  << num_channels_per_link.at(i_link) << " channels"
                  << std::endl;
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
        CRC link_crc;
        r.next();
        fpga_crc(r.now());
        link_crc(r.now());
        std::cout << std::bitset<32>(r.now()) << std::endl;
        uint32_t roc_id{(r.now() >> 8 + 5 + 1) & mask<16>::m};
        bool crc_ok{(r.now() >> 8 + 5) & mask<1>::m == 1};

        // get readout map from the last 8 bits of this word
        // and the entire next word
        std::bitset<40> ro_map{r.now() & mask<8>::m};
        ro_map <<= 32;
        r.next();
        fpga_crc(r.now());
        link_crc(r.now());
        ro_map |= r.now();

        std::cout << "Start looping through channels..." << std::endl;
        // loop through channels on this link,
        //  since some channels may have been suppressed because of low
        //  amplitude the channel ID is not the same as the index it
        //  is listed in.
        int channel_id{-1};
        for (uint32_t j{0}; j < num_channels_per_link.at(i_link); j++) {
          // skip zero-suppressed channel IDs
          do {
            channel_id++;
          } while (channel_id < 40 and not ro_map.test(channel_id));

          // next word is this channel
          r.next();
          fpga_crc(r.now());
          std::cout << std::bitset<32>(r.now());

          if (channel_id == 0) {
            /** Special "Header" Word from ROC
             * 0101 | BXID (12) | RREQ (6) | OR (3) | HE (3) | 0101
             */
            std::cout << " : ROC Header";
            link_crc(r.now());
            uint32_t bx_id{(r.now() >> 4 + 3 + 3 + 6) & mask<12>::m};
            uint32_t short_event{(r.now() >> 4 + 3 + 3) & mask<6>::m};
            uint32_t short_orbit{(r.now() >> 4 + 3) & mask<3>::m};
            uint32_t hamming_errs{(r.now() >> 4) & mask<3>::m};
          } else if (channel_id == common_mode_channel) {
            /** Common Mode Channels
             * 10 | 0000000000 | Common Mode ADC 0 (10) | Common Mode ADC 1 (10)
             */
            link_crc(r.now());
            std::cout << " : Common Mode";
          } else if (channel_id == 39) {
            // CRC checksum from ROC
            uint32_t crc{r.now()};
            std::cout << " : CRC checksum  : ";
            std::cout << std::hex << link_crc.get() << " =? ";
            std::cout << crc << std::dec;
            if (link_crc.get() != crc) {
              EXCEPTION_RAISE("BadCRC",
                  "Our calculated link checksum doesn't match the one from raw data.");
            }
          } else {
            /// DAQ Channels

            link_crc(r.now());
            /** Generate Packed Electronics ID
             *   Link Index i_link
             *   Channel ID channel_id
             *   ROC ID     roc_id
             *   FPGA ID    fpga
             * are all available.
             * For now, we just generate a dummy mapping
             * using the link and channel indices.
             */
            uint32_t eid{i_link * 100 + channel_id};

            // copy data into EID->sample map
            data[eid].emplace_back(r.now());
            std::cout << " : DAQ Channel";
          }  // type of channel
          std::cout << std::endl;
        }  // loop over channels (j in Table 4)
        std::cout << "done looping through channels" << std::endl;
      }  // loop over links

      // another CRC checksum from FPGA
      r.next();
      uint32_t crc{r.now()};
      std::cout << "FPGA Checksum : " << std::hex << fpga_crc.get()
        << " =? " << crc << std::dec << std::endl;
      if (fpga_crc.get() != crc) {
        EXCEPTION_RAISE("BadCRC",
            "Our calculated FPGA checksum doesn't match the one read in.");
      }
    } catch (std::out_of_range& oor) {
      std::cout << oor.what() << std::endl;
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
