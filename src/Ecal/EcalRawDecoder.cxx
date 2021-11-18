#include "Ecal/EcalRawDecoder.h"

#include <bitset>
#include <iomanip>
#include <optional>

#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/EcalID.h"
#include "Ecal/EcalDetectorMap.h"
#include "Packing/Utility/BufferReader.h"
#include "Packing/Utility/CRC.h"
#include "Packing/Utility/Hex.h"
#include "Packing/Utility/Mask.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace ecal {

// we are using 32-bit words here
using hex = packing::utility::hex<uint32_t>;
using BufferReader = packing::utility::BufferReader<uint32_t>;

void EcalRawDecoder::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  roc_version_ = ps.getParameter<int>("roc_version");
  translate_eid_ = ps.getParameter<bool>("translate_eid");
}

void EcalRawDecoder::produce(framework::Event& event) {
  /**
   * Static parameters depending on ROC version
   */
  static const unsigned int common_mode_channel = roc_version_ == 2 ? 19 : 1;
  /// words for reading and decoding
  static uint32_t head1, head2, w;

  BufferReader reader{event.getCollection<uint8_t>(input_name_, input_pass_)};

  /** Re-sort the data from grouped by bunch to by channel
   *
   * The readout cip streams the data off of it, so it doesn't
   * have time to re-group the signals across multiple bunches (samples)
   * by their channel ID. We need to do that here.
   */
  // fill map of **electronic** IDs to the digis that were read out
  std::map<ldmx::EcalElectronicsID,
           std::vector<ldmx::HgcrocDigiCollection::Sample>>
      eid_to_samples;
  while (reader >> head1 >> head2) {
    /// are we reading a buffer from multi-sample per event?
    if (head1 == 0x11111111 and head2 == 0xbeef2021) {
      /* whole event header word looks like
       *
       * VERSION (4) | FPGA ID (8) | NSAMPLES (4) | LEN (16)
       */
      uint32_t whole_event_header;
      reader >> whole_event_header;
      uint32_t version = (whole_event_header >> 28) & packing::utility::mask<4>;
      uint32_t fpga = (whole_event_header >> 20) & packing::utility::mask<8>;
      uint32_t nsamples =
          (whole_event_header >> 16) & packing::utility::mask<4>;
      uint32_t eventlen = whole_event_header & packing::utility::mask<16>;

      // sample counters
      std::vector<uint32_t> length_per_sample(nsamples, 0);
      for (uint32_t i_sample{0}; i_sample < nsamples; i_sample++) {
        if (i_sample % 2 == 0) {
          reader >> w;
        }
        uint32_t shift_in_word = 16 * (i_sample % 2);
        length_per_sample[i_sample] =
            (w >> shift_in_word) & packing::utility::mask<12>;
      }

      // read first sample headers
      reader >> head1 >> head2;
    } else if (head1 == 0xd07e2021 and head2 == 0x12345678) {
      // these are the special footer words at the end,
      //  done with event
      break;
    }

    /** Decode Bunch Header
     * We have a few words of header material before the actual data.
     * This header material is assumed to be encoded as in Table 3
     * of the DAQ specs.
     *
     * <name> (bits)
     *
     * VERSION (4) | FPGA_ID (8) | NLINKS (6) | 00 | LEN (12)
     * BX ID (12) | RREQ (10) | OR (10)
     * RID ok (1) | CRC ok (1) | LEN3 (6) |
     *  RID ok (1) | CRC ok (1) | LEN2 (6) |
     *  RID ok (1) | CRC ok (1) | LEN1 (6) |
     *  RID ok (1) | CRC ok (1) | LEN0 (6)
     * ... other listing of links ...
     */
    packing::utility::CRC fpga_crc;
    fpga_crc << head1;
    std::cout << hex(head1) << " : ";
    uint32_t version = (head1 >> 28) & packing::utility::mask<4>;
    // std::cout << "version " << version << std::flush;
    uint32_t one{1};
    if (version != one)
      EXCEPTION_RAISE("VersMis",
                      "EcalRawDecoder only knows version 1 of DAQ format.");

    uint32_t fpga = (head1 >> 20) & packing::utility::mask<8>;
    uint32_t nlinks = (head1 >> 14) & packing::utility::mask<6>;
    uint32_t len = head1 & packing::utility::mask<12>;

    // std::cout << ", fpga: " << fpga << ", nlinks: " << nlinks << ", len: " <<
    // len << std::endl;
    fpga_crc << head2;
    // std::cout << hex(head2) << " : ";

    uint32_t bx_id = (head2 >> 20) & packing::utility::mask<12>;
    uint32_t rreq = (head2 >> 10) & packing::utility::mask<10>;
    uint32_t orbit = head2 & packing::utility::mask<10>;

    // std::cout << "bx_id: " << bx_id << ", rreq: " << rreq << ", orbit: " <<
    // orbit << std::endl;

    std::vector<uint32_t> length_per_link(nlinks, 0);
    for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
      if (i_link % 4 == 0) {
        reader >> w;
        fpga_crc << w;
        // std::cout << hex(w) << " : Four Link Pack " << std::endl;
      }
      uint32_t shift_in_word = 8 * (i_link % 4);
      bool rid_ok = (w >> (shift_in_word + 7)) & packing::utility::mask<1> == 1;
      bool cdc_ok = (w >> (shift_in_word + 6)) & packing::utility::mask<1> == 1;
      length_per_link[i_link] =
          (w >> shift_in_word) & packing::utility::mask<6>;
      // std::cout << "  Link " << i_link << " readout " <<
      // length_per_link.at(i_link) << " channels" << std::endl;
    }

    /** Decode Each Link in Sequence
     * Now we should be decoding each link serially
     * where each link was encoded as in Table 4 of
     * the DAQ specs
     *
     * ROC_ID (16) | CRC ok (1) | 0 (7) | RO Map (8)
     * RO Map (32)
     */

    for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
      // move on from last word counting links or previous link
      // std::cout << "RO Link " << i_link << std::endl;
      packing::utility::CRC link_crc;
      reader >> w;
      fpga_crc << w;
      link_crc << w;
      uint32_t roc_id = (w >> 16) & packing::utility::mask<16>;
      bool crc_ok = (w >> 15) & packing::utility::mask<1> == 1;
      // std::cout << hex(w) << " : roc_id " << roc_id << ", crc_ok (v2
      // always false) " << std::boolalpha << crc_ok << std::endl;

      // get readout map from the last 8 bits of this word
      // and the entire next word
      std::bitset<40> ro_map = w & packing::utility::mask<8>;
      ro_map <<= 32;
      reader >> w;
      fpga_crc << w;
      link_crc << w;
      ro_map |= w;

      // std::cout << "Start looping through channels..." << std::endl;
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
        reader >> w;
        fpga_crc << w;
        // std::cout << hex(w) << " " << channel_id;

        if (channel_id == 0) {
          /** Special "Header" Word from ROC
           *
           * version 3:
           * 0101 | BXID (12) | RREQ (6) | OR (3) | HE (3) | 0101
           *
           * version 2:
           * 10101010 | BXID (12) | WADD (9) | 1010
           */
          // std::cout << " : ROC Header";
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
          // std::cout << " : Common Mode";
        } else if (channel_id == 39) {
          // CRC checksum from ROC
          uint32_t crc = w;
          // std::cout << " : CRC checksum  : " << hex(link_crc.get()) <<
          // " =? " << hex(crc);
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

          // std::cout << " : DAQ Channel ";

          // std::cout << fpga << " " << roc_id-256 << " " << channel_id << " ";
          /**
           * The subfields for the electronics ID infrastructure need to start
           * from 0 and count up. This means we need to subtract some of the
           * fields by their lowest value before inputting them into the EID.
           *
           * TODO fix hardcoded starting value
           *
           *  roc_id-256 is the ssame as i_link = is this a coincidence?
           *  or should we change the second input to be the link index
           */
          ldmx::EcalElectronicsID eid(fpga - 1, roc_id - 256, channel_id);
          // std::cout << eid.index();

          // copy data into EID->sample map
          eid_to_samples[eid].emplace_back(w);
        }  // type of channel
        // std::cout << std::endl;
      }  // loop over channels (j in Table 4)
      // std::cout << "done looping through channels" << std::endl;
    }  // loop over links

    // another CRC checksum from FPGA
    reader >> w;
    uint32_t crc = w;
    // std::cout << "FPGA Checksum : " << hex(fpga_crc.get()) << " =? "
    // << hex(crc) << std::endl;
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
        getCondition<EcalDetectorMap>(EcalDetectorMap::CONDITIONS_OBJECT_NAME)};
    for (auto const& [eid, digi] : eid_to_samples) {
      // The electronics map returns an empty ID of the correct
      // type when the electronics ID is not found.
      //  need to check if the electronics ID exists
      //  TODO: do we want to end processing if this happens?
      if (detmap.exists(eid)) {
        uint32_t did_raw = detmap.get(eid).raw();
        digis.addDigi(did_raw, digi);
      } else {
        /** DO NOTHING
         *  skip hits where the EID aren't in the detector mapping
         *  no zero supp during test beam on the front-end,
         *  so channels that aren't connected to anything are still
         *  being readout.
        std::cout << "EID(" << eid.fiber() << "," << eid.elink() << "," <<
        eid.channel() << ") "; for (auto& s : digi) std::cout <<
        hex(s.raw()) << " "; std::cout << std::endl;
         */
      }
    }
  } else {
    /**
     * no EID translation, just add the digis to the digi collection
     * with their raw electronic ID
     * TODO: remove this, we shouldn't be able to get past
     *       the decoding stage without translating the EID
     *       into a detector ID to avoid confusion in recon
     */
    for (auto const& [eid, digi] : eid_to_samples) {
      digis.addDigi(eid.raw(), digi);
    }
  }

  // std::cout << "adding " << digis.getNumDigis() << " digis each with " <<
  // digis.getNumSamplesPerDigi() << " samples to event bus" << std::endl;
  event.add(output_name_, digis);
  return;
}  // produce

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalRawDecoder);
