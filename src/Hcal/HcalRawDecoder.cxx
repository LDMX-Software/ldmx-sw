#include "Hcal/HcalRawDecoder.h"

#include <bitset>
#include <iomanip>
#include <optional>

#include "DetDescr/HcalElectronicsID.h"
#include "DetDescr/HcalID.h"
#include "Hcal/HcalDetectorMap.h"
#include "Packing/Utility/BufferReader.h"
#include "Packing/Utility/CRC.h"
#include "Packing/Utility/Mask.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace hcal {

namespace utility {

/**
 * Read out 32-bit words from a buffer.
 */
class Reader {
  const std::vector<uint32_t> &buffer_;
  std::size_t i_word_;
  uint32_t next() {
    uint32_t w{buffer_.at(i_word_)};
    i_word_++;
    return w;
  }
 public:
  Reader(const std::vector<uint32_t>& b) : buffer_{b}, i_word_{0} {}
  operator bool() {
    return (i_word_ < buffer_.size());
  }
  Reader& operator>>(uint32_t& w) {
    if (*this)
      w = next();
    return *this;
  }
};  // Reader

}  // namespace utility

namespace debug {

struct hex {
  uint32_t word_;
  hex(uint32_t w) : word_{w} {}
};

}  // namespace debug

inline std::ostream& operator<<(std::ostream& os, const debug::hex& h) {
  os << "0x" << std::setfill('0') << std::setw(8) << std::hex << h.word_
     << std::dec;
  return os;
}

void HcalRawDecoder::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  roc_version_ = ps.getParameter<int>("roc_version");
  translate_eid_ = ps.getParameter<bool>("translate_eid");
}

void HcalRawDecoder::produce(framework::Event& event) {
  /**
   * Static parameters depending on ROC version
   */
  static const unsigned int common_mode_channel = roc_version_ == 2 ? 19 : 1;
  /// words for reading and decoding
  static uint32_t head1, head2, w;

  utility::Reader reader_{event.getCollection<uint32_t>(input_name_, input_pass_)};

  /** Re-sort the data from grouped by bunch to by channel
   *
   * The readout cip streams the data off of it, so it doesn't
   * have time to re-group the signals across multiple bunches (samples)
   * by their channel ID. We need to do that here.
   */
  // fill map of **electronic** IDs to the digis that were read out
  std::map<ldmx::HcalElectronicsID,
           std::vector<ldmx::HgcrocDigiCollection::Sample>>
      eid_to_samples;
  while (reader_ >> head1 >> head2) {
    /** Decode Bunch Header
     * We have a few words of header material before the actual data.
     * This header material is assumed to be encoded as in Table 3
     * of the DAQ specs.
     *
     * <name> (bits)
     *
     * VERSION (4) | FPGA_ID (8) | NLINKS (6) | 00 | LEN (12)
     * BX ID (12) | RREQ (10) | OR (10)
     * RID ok (1) | CDC ok (1) | LEN3 (6) |
     *  RID ok (1) | CDC ok (1) | LEN2 (6) |
     *  RID ok (1) | CDC ok (1) | LEN1 (6) |
     *  RID ok (1) | CDC ok (1) | LEN0 (6)
     * ... other listing of links ...
     */
    packing::utility::CRC fpga_crc;
    fpga_crc << head1;
    std::cout << debug::hex(head1) << " : ";
    uint32_t version = (head1 >> 28) & packing::utility::mask<4>;
    std::cout << "version " << version << std::flush;
    uint32_t one{1};
    if (version != one)
      EXCEPTION_RAISE("VersMis",
                      "HcalRawDecoder only knows version 1 of DAQ format.");

    uint32_t fpga = (head1 >> 20) & packing::utility::mask<8>;
    uint32_t nlinks = (head1 >> 14) & packing::utility::mask<6>;
    uint32_t len = head1 & packing::utility::mask<12>;

    std::cout << ", fpga: " << fpga << ", nlinks: " << nlinks
              << ", len: " << len << std::endl;
    fpga_crc << head2;
    std::cout << debug::hex(head2) << " : ";

    uint32_t bx_id = (head2 >> 20) & packing::utility::mask<12>;
    uint32_t rreq = (head2 >> 10) & packing::utility::mask<10>;
    uint32_t orbit = head2 & packing::utility::mask<10>;

    std::cout << "bx_id: " << bx_id << ", rreq: " << rreq
              << ", orbit: " << orbit << std::endl;

    std::vector<uint32_t> length_per_link(nlinks, 0);
    for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
      if (i_link % 4 == 0) {
        reader_ >> w;
        fpga_crc << w;
        std::cout << debug::hex(w) << " : Four Link Pack " << std::endl;
      }
      uint32_t shift_in_word = 8 * (i_link % 4);
      bool rid_ok = (w >> (shift_in_word + 7)) & packing::utility::mask<1> == 1;
      bool cdc_ok = (w >> (shift_in_word + 6)) & packing::utility::mask<1> == 1;
      length_per_link[i_link] =
          (w >> shift_in_word) & packing::utility::mask<6>;
      std::cout << "  Link " << i_link << " readout "
                << length_per_link.at(i_link) << " channels" << std::endl;
    }

    /** Decode Each Link in Sequence
     * Now we should be decoding each link serially
     * where each link was encoded as in Table 4 of
     * the DAQ specs
     *
     * 0 (7) | CRC ok (1) | ROC_ID (8) |  0 (8) | RO Map (8)
     * RO Map (32)
     */

    for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
      // move on from last word counting links or previous link
      std::cout << "RO Link " << i_link << std::endl;
      packing::utility::CRC link_crc;
      reader_ >> w;
      fpga_crc << w;
      link_crc << w;
      uint32_t roc_id = (w >> 16) & packing::utility::mask<16>;
      bool crc_ok = (w >> 15) & packing::utility::mask<1> == 1;
      std::cout << debug::hex(w) << " : roc_id " << roc_id << ", crc_ok (v2 always false) "
                << std::boolalpha << crc_ok << std::endl;

      // get readout map from the last 8 bits of this word
      // and the entire next word
      std::bitset<40> ro_map = w & packing::utility::mask<8>;
      ro_map <<= 32;
      reader_ >> w;
      fpga_crc << w;
      link_crc << w;
      ro_map |= w;

      std::cout << "Start looping through channels..." << std::endl;
      // loop through channels on this link,
      //  since some channels may have been suppressed because of low
      //  amplitude the channel ID is not the same as the index it
      //  is listed in.
      int channel_id{-1};
      for (uint32_t j{0}; j < length_per_link.at(i_link) - 2; j++) {
        // skip zero-suppressed channel IDs
        do {
          channel_id++;
        } while (channel_id < 40 and not ro_map.test(channel_id));

        // next word is this channel
        reader_ >> w;
        fpga_crc << w;
        std::cout << debug::hex(w) << " " << channel_id;

        if (channel_id == 0) {
          /** Special "Header" Word from ROC
           *
           * version 3:
           * 0101 | BXID (12) | RREQ (6) | OR (3) | HE (3) | 0101
           *
           * version 2:
           * 10101010 | BXID (12) | WADD (9) | 1010
           */
          std::cout << " : ROC Header";
          link_crc << w;
          uint32_t bx_id = (w >> 16) & packing::utility::mask<12>;
          uint32_t short_event = (w >> 10) & packing::utility::mask<6>;
          uint32_t short_orbit = (w >> 7) & packing::utility::mask<3>;
          uint32_t hamming_errs = (w >> 4) & packing::utility::mask<3>;
        } else if (channel_id == common_mode_channel) {
          /** Common Mode Channels
           * 10 | 0000000000 | Common Mode ADC 0 (10) | Common Mode ADC 1 (10)
           */
          link_crc << w;
          std::cout << " : Common Mode";
        } else if (channel_id == 39) {
          // CRC checksum from ROC
          uint32_t crc = w;
          std::cout << " : CRC checksum  : " << debug::hex(link_crc.get())
                    << " =? " << debug::hex(crc);
          /*
          if (link_crc.get() != crc) {
            EXCEPTION_RAISE("BadCRC",
                            "Our calculated link checksum doesn't match the "
                            "one from raw data.");
          }
          */
        } else {
          /// DAQ Channels

          link_crc << w;
          /** Generate Packed Electronics ID
           *   Link Index i_link
           *   Channel ID channel_id
           *   ROC ID     roc_id
           *   FPGA ID    fpga
           * are all available.
           * For now, we just generate a dummy mapping
           * using the link and channel indices.
           */

          std::cout << " : DAQ Channel ";

          std::cout << fpga << " " << roc_id << " " << channel_id << " ";
          // TODO fix hardcoded starting value
          ldmx::HcalElectronicsID eid(fpga, roc_id-256, channel_id);
          std::cout << eid.index();

          // copy data into EID->sample map
          eid_to_samples[eid].emplace_back(w);
        }  // type of channel
        std::cout << std::endl;
      }  // loop over channels (j in Table 4)
      std::cout << "done looping through channels" << std::endl;
    }  // loop over links

    // another CRC checksum from FPGA
    reader_ >> w;
    uint32_t crc = w;
    std::cout << "FPGA Checksum : " << debug::hex(fpga_crc.get()) << " =? "
              << debug::hex(crc) << std::endl;
    /* TODO
     *  fix calculation of FPGA checksum
     *  I can't figure out why it isn't matching, but there
     *  is definitely a word here where the FPGA checksum would be.
    if (fpga_crc.get() != crc) {
      EXCEPTION_RAISE(
          "BadCRC",
          "Our calculated FPGA checksum doesn't match the one read in.");
    }
    */
  }

  // check if there was any data to decode
  //  helpful for when reading directly from a raw data file
  //  so we can simply overestimate the number of events
  if (eid_to_samples.size() == 0) {
    abortEvent();
  }

  ldmx::HgcrocDigiCollection digis;
  // assume all channels have same number of samples
  digis.setNumSamplesPerDigi(eid_to_samples.begin()->second.size());
  digis.setSampleOfInterestIndex(0);  // TODO configurable
  digis.setVersion(roc_version_);
  if (translate_eid_) {
    /**
     * Translation
     *
     * Now the HgcrocDigiCollection::Sample class handles the
     * unpacking of individual samples; however, we still need
     * to translate electronic IDs into detector IDs.
     */
    auto detmap{
        getCondition<HcalDetectorMap>(HcalDetectorMap::CONDITIONS_OBJECT_NAME)};
    for (auto const& [eid, digi] : eid_to_samples) {
      // The electronics map returns an empty ID of the correct
      // type when the electronics ID is not found.
      //  need to check if the electronics ID exists
      //  TODO: do we want to end processing if this happens?
      if (detmap.exists(eid)) {
        uint32_t did_raw = detmap.get(eid).raw();
        digis.addDigi(did_raw, digi);
      } else {
        // DO NOTHING
        //  skip hits where the EID aren't in the detector mapping
        //  DO WE ACTUALLY WANT TO DO THIS?
      }
    }
  } else {
    // no EID translation, just add the digis to the digi collection
    // with their raw electronic ID
    for (auto const& [eid, digi] : eid_to_samples) {
      digis.addDigi(eid.raw(), digi);
    }
  }

  std::cout << "adding " << digis.getNumDigis() << " digis to event bus" << std::endl;
  event.add(output_name_, digis);
  return;
}  // produce

}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalRawDecoder);
