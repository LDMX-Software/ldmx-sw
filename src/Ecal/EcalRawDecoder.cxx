#include <bitset>

#include "Ecal/EcalRawDecoder.h"

#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/EcalID.h"
#include "Ecal/EcalDetectorMap.h"
#include "Packing/Utility/Mask.h"
#include "Packing/Utility/CRC.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Tools/BufferReader.h"

namespace ecal {

EcalRawDecoder::EcalRawDecoder(const std::string& name,
                               framework::Process& process)
    : Producer(name, process) {}

EcalRawDecoder::~EcalRawDecoder() {}

void EcalRawDecoder::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  roc_version_ = ps.getParameter<int>("roc_version");
}

void EcalRawDecoder::produce(framework::Event& event) {
  /**
   * Static parameters depending on ROC version
   */
  static const unsigned int common_mode_channel = roc_version_ == 2 ? 19 : 1;

  /** Re-sort the data from grouped by bunch to by channel
   * The readout chip streams the data off of it, so it doesn't
   * have time to re-group the signals across multiple bunches (samples)
   * by their channel ID. We need to do that here.
   */
  // fill map of **electronic** IDs to the digis that were read out
  std::map<ldmx::EcalElectronicsID,
           std::vector<ldmx::HgcrocDigiCollection::Sample>>
      eid_to_samples;
  tools::BufferReader<uint32_t, uint32_t> r{
      event.getCollection<uint32_t>(input_name_, input_pass_)};
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
      packing::utility::CRC fpga_crc;
      fpga_crc << r.now();
      std::cout << std::bitset<32>(r.now()) << " : ";
      uint32_t version =
          (r.now() >> 12 + 1 + 6 + 8) & packing::utility::mask<4>;
      std::cout << "version " << version << std::flush;
      uint32_t one{1};
      if (version != one)
        EXCEPTION_RAISE("VersMis", "Hgcroc Translator only knows version 1.");

      uint32_t fpga = (r.now() >> 12 + 1 + 6) & packing::utility::mask<8>;
      uint32_t nlinks = (r.now() >> 12 + 1) & packing::utility::mask<6>;
      uint32_t len = r.now() & packing::utility::mask<12>;

      std::cout << ", fpga: " << fpga << ", nlinks: " << nlinks
                << ", len: " << len << std::endl;
      r.next();
      fpga_crc << r.now();
      std::cout << std::bitset<32>(r.now()) << " : ";

      uint32_t bx_id = (r.now() >> 10 + 10) & packing::utility::mask<12>;
      uint32_t rreq = (r.now() >> 10) & packing::utility::mask<10>;
      uint32_t orbit = r.now() & packing::utility::mask<10>;

      std::cout << "bx_id: " << bx_id << ", rreq: " << rreq
                << ", orbit: " << orbit << std::endl;
      std::vector<uint32_t> num_channels_per_link(nlinks, 0);
      for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
        if (i_link % 4 == 0) {
          r.next();
          fpga_crc << r.now();
        }
        uint32_t shift_in_word = 8 * i_link % 4;
        bool rid_ok =
            (r.now() >> shift_in_word + 7) & packing::utility::mask<1> == 1;
        bool cdc_ok =
            (r.now() >> shift_in_word + 6) & packing::utility::mask<1> == 1;
        num_channels_per_link[i_link] =
            (r.now() >> shift_in_word) & packing::utility::mask<6>;
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
        packing::utility::CRC link_crc;
        r.next();
        fpga_crc << r.now();
        link_crc << r.now();
        uint32_t roc_id = (r.now() >> 8 + 5 + 1) & packing::utility::mask<16>;
        bool crc_ok = (r.now() >> 8 + 5) & packing::utility::mask<1> == 1;
        std::cout << std::bitset<32>(r.now()) << " : roc_id " << roc_id
                  << ", cfc_ok " << std::boolalpha << crc_ok << std::endl;

        // get readout map from the last 8 bits of this word
        // and the entire next word
        std::bitset<40> ro_map = r.now() & packing::utility::mask<8>;
        ro_map <<= 32;
        r.next();
        fpga_crc << r.now();
        link_crc << r.now();
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
          fpga_crc << r.now();
          std::cout << std::bitset<32>(r.now());

          if (channel_id == 0) {
            /** Special "Header" Word from ROC
             * 0101 | BXID (12) | RREQ (6) | OR (3) | HE (3) | 0101
             */
            std::cout << " : ROC Header";
            link_crc << r.now();
            uint32_t bx_id =
                (r.now() >> 4 + 3 + 3 + 6) & packing::utility::mask<12>;
            uint32_t short_event =
                (r.now() >> 4 + 3 + 3) & packing::utility::mask<6>;
            uint32_t short_orbit =
                (r.now() >> 4 + 3) & packing::utility::mask<3>;
            uint32_t hamming_errs = (r.now() >> 4) & packing::utility::mask<3>;
          } else if (channel_id == common_mode_channel) {
            /** Common Mode Channels
             * 10 | 0000000000 | Common Mode ADC 0 (10) | Common Mode ADC 1 (10)
             */
            link_crc << r.now();
            std::cout << " : Common Mode";
          } else if (channel_id == 39) {
            // CRC checksum from ROC
            uint32_t crc = r.now();
            std::cout << " : CRC checksum  : ";
            std::cout << std::hex << link_crc.get() << " =? ";
            std::cout << crc << std::dec;
            if (link_crc.get() != crc) {
              EXCEPTION_RAISE("BadCRC",
                              "Our calculated link checksum doesn't match the "
                              "one from raw data.");
            }
          } else {
            /// DAQ Channels

            link_crc << r.now();
            /** Generate Packed Electronics ID
             *   Link Index i_link
             *   Channel ID channel_id
             *   ROC ID     roc_id
             *   FPGA ID    fpga
             * are all available.
             * For now, we just generate a dummy mapping
             * using the link and channel indices.
             */
            ldmx::EcalElectronicsID eid(fpga, roc_id, channel_id);

            // copy data into EID->sample map
            eid_to_samples[eid].emplace_back(r.now());
            std::cout << " : DAQ Channel";
          }  // type of channel
          std::cout << std::endl;
        }  // loop over channels (j in Table 4)
        std::cout << "done looping through channels" << std::endl;
      }  // loop over links

      // another CRC checksum from FPGA
      r.next();
      uint32_t crc = r.now();
      std::cout << "FPGA Checksum : " << std::hex << fpga_crc.get() << " =? "
                << crc << std::dec << std::endl;
      if (fpga_crc.get() != crc) {
        EXCEPTION_RAISE(
            "BadCRC",
            "Our calculated FPGA checksum doesn't match the one read in.");
      }
    } catch (std::out_of_range& oor) {
      std::cout << oor.what() << std::endl;
      EXCEPTION_RAISE("MisFormat",
                      "Recieved raw data that was not formatted correctly.");
    }
  } while (r.next(false));

  /**
   * Translation
   *
   * Now the HgcrocDigiCollection::Sample class handles the
   * unpacking of individual samples; however, we still need
   * to translate electronic IDs into detector IDs.
   */
  auto detmap{
      getCondition<EcalDetectorMap>(EcalDetectorMap::CONDITIONS_OBJECT_NAME)};
  ldmx::HgcrocDigiCollection digis;
  for (auto const& [eid, digi] : eid_to_samples) {
    // TODO: This checking of existence should be temporary,
    //       the electronic ID mapping should be complete.
    uint32_t did_raw{eid.raw()};
    if (detmap.exists(eid)) {
      did_raw = detmap.get(eid).raw();
    }

    digis.addDigi(did_raw, digi);
  }

  event.add(output_name_, digis);

  return;
}  // produce

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalRawDecoder);
