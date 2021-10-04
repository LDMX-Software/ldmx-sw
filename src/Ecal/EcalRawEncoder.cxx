#include <bitset>

#include "Ecal/EcalRawEncoder.h"

#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/EcalID.h"
#include "Ecal/EcalDetectorMap.h"
#include "Packing/Utility/BufferReader.h"
#include "Packing/Utility/Mask.h"
#include "Packing/Utility/CRC.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace ecal {

EcalRawEncoder::EcalRawEncoder(const std::string& name,
                               framework::Process& process)
    : Producer(name, process) {}

EcalRawEncoder::~EcalRawEncoder() {}

void EcalRawEncoder::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  roc_version_ = ps.getParameter<int>("roc_version");
}

void EcalRawEncoder::produce(framework::Event& event) {
  /**
   * Static parameters depending on ROC version
   */
  static const unsigned int common_mode_channel = roc_version_ == 2 ? 19 : 1;

  auto digis{event.getObject<ldmx::HgcrocDigiCollection>(input_name_, input_pass_)};
  std::vector<
    std::vector<
      std::vector<
        std::map<uint32_t,uint32_t> // channel to sample
        > // links
      > // fpgas
    > // bunches
    sorted_samples(digis.getNumSamplesPerDigi());
  /** 
   * TODO need to resize to fit constraints on number of links and fpgas
  for (auto& bunch : sorted_samples) {
    bunch.resize(total_num_possible_fpga);
    for (auto& fpga : bunch) {
      fpga.resize(total_num_possible_links_per_fpga);
      // channel to sample mapping we want to stay empty
    }
  }
  */
  
  /**
   * Translation
   *
   * Now the HgcrocDigiCollection::Sample class handles the
   * unpacking of individual samples; however, we still need
   * to translate detector IDs into electronics ID and resort
   * the data into grouped by bunch.
   */
  auto detmap{
      getCondition<EcalDetectorMap>(EcalDetectorMap::CONDITIONS_OBJECT_NAME)};
  for (auto digi : digis) {
    ldmx::EcalID detid{digi.id()};
    ldmx::EcalElectronicsID eid{detmap.get(detid)};

    std::size_t i_bx{0};
    for (auto sample : digi) {
      sorted_samples[i_bx][eid.fiber()][eid.elink()][eid.channel()] = sample.raw();
      i_bx++;
    }
  }


  /**
   * Encoding
   *
   * Now that the samples are sorted into per-bunch groupings,
   * we can start writing this data into the encoded data format
   * documented in the ECal DAQ specifications. Since the class
   * HgcrocDigiCollection::Sample handles the encoding and decoding
   * of specific sample words, we "only" need to actually encode
   * the header information the calculate the CRC checksums.
   */
  std::vector<uint32_t> buffer;
  for (auto const& bunch : sorted_samples) {
    // bunch lists the fpgs, links, and channels with their corresponding sample
    for (auto const& fpga : bunch) {
      /** Encode Bunch Header
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
      // fpga lists the links and channels with their corresponding sample
      for (auto const& link : fpga) {
        /** Encode Each Link in Sequence
         * Now we should be decoding each link serially
         * where each link was encoded as in Table 4 of
         * the DAQ specs
         *
         * ROC_ID (16) | CRC ok (1) | 00000 | RO Map (8)
         * RO Map (32)
         */
        packing::utility::CRC link_crc;

        // each link maps the channels that were readout to their sample
        for (auto const& [channel, sample] : link) {

        }
      }
    }
  }

  event.add(output_name_, buffer);

  return;
}  // produce

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalRawEncoder);
