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
    std::map<uint16_t,
      std::map<uint16_t,
        std::map<uint32_t,uint32_t> // channel to sample
        > // links
      > // fpgas
    > // bunches
    sorted_samples(digis.getNumSamplesPerDigi());
  
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
  static uint32_t word; // word to use for constructing buffer
  uint32_t i_bx{0};
  for (auto const& bunch : sorted_samples) {
    /**TODO calculate bunch ID, read request, and orbit from sample ID, event number, and run number 
     * placeholder: 
     *  bunch ID = event number 
     *  read request = sample ID
     *  orbit = run number
     */
    uint32_t bunch_id = event.getEventNumber();
    uint32_t rreq = i_bx;
    uint32_t orbit = event.getEventHeader().getRun();

    // bunch lists the fpgs, links, and channels with their corresponding sample
    for (auto const& [fpga_id, links] : bunch) {
      /**
       * Calculate lengths of link sub-packets
       *
       * Table 4 of ECal DAQ Specifications.
       *
       * Each ROC link has 3 header words, a common mode channel, and
       * a trailing CRC checksum word. The 3 header words contain a readout map
       * of which channels are included in the DAQ packet, so we end up with.
       *
       *  len of link = 3 + 1 + channels.size() + 1;
       */
      std::vector<uint32_t> link_lengths;
      for (auto const& [link_id, channels] : links) {
        link_lengths.push_back(3+1+channels.size()+1);
      }

      /**
       * The total FPGA packet includes at least 2 header words, a trailing checksum word,
       * and a single word for each four links.
       *
       * This means we add up the subpacket lengths into subpacket_total
       * and then
       *
       *  n_linkwords = (nlinks/4+(nlinks%4!=0))
       *  total_length = 2 + n_linkwords + subpacket_total + 1;
       */
      uint32_t n_linkwords = link_lengths.size()/4 + (link_lengths.size()%4 != 0);
      uint32_t total_length{2 + n_linkwords + 1};
      for (uint32_t const& link_len : link_lengths) {
        total_length += link_len;
      }

      packing::utility::CRC fpga_crc;
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
      word = 0;
      word |= (1 << 12+1+6+8); // version
      word |= (fpga_id & packing::utility::mask<8>) << 12+1+6; // FPGA
      word |= (links.size() & packing::utility::mask<6>) << 12+1; // NLINKS
      word |= (total_length & packing::utility::mask<12>); // LEN TODO
      buffer.push_back(word);
      fpga_crc << word;

      word = 0;
      word |= (bunch_id & packing::utility::mask<12>) << 20; // BX ID
      word |= (rreq & packing::utility::mask<10>) << 10; // RREQ
      word |= (orbit & packing::utility::mask<10>); // OR
      buffer.push_back(word);
      fpga_crc << word;

      /**
       * Encode lengths of link subpackets
       */
      for (uint32_t i_linkword{0}; i_linkword < n_linkwords; i_linkword++) {
        word = 0;
        for (uint32_t i_linklen{0}; i_linklen < 4; i_linklen++) {
          uint32_t i_link = 4*i_linkword + i_linklen;
          if (i_link <= link_lengths.size()) {
            // we have a link
            word |= (((0b11 << 6) + (link_lengths.at(i_link) & packing::utility::mask<6>)) << 8*i_linklen);
          } // do we have a link for this linklen subword?
        }   // loop through subwords in this word
        buffer.push_back(word);
        fpga_crc << word;
      } // loop through words

      // fpga lists the links and channels with their corresponding sample
      for (auto const& [link_id, channels] : links) {
        /**
         * Prepare RO Map bitset
         */
        std::bitset<40> ro_map; //starts as all 0s
        ro_map.set(0); // special "header" word from ROC
        ro_map.set(39); // trailing checksum from ROC
        // each link maps the channels that were readout to their sample
        for (auto const& [channel, sample] : channels) {
          ro_map.set(channel);
        }

        packing::utility::CRC link_crc;
        /** Encode Each Link in Sequence
         * Now we should be decoding each link serially
         * where each link was encoded as in Table 4 of
         * the DAQ specs
         *
         * ROC_ID (16) | CRC ok (1) | 00000 | RO Map (8)
         * RO Map (32)
         */

        word = 0;
        word |= (link_id & packing::utility::mask<16>) << 16;
        word |= 1 << 15;
        // put first 8bits of RO Map in first header word
        word |= (ro_map >> 32).to_ulong();

        buffer.push_back(word);
        fpga_crc << word;
        link_crc << word;

        // next header word is end of RO map
        word = (ro_map.to_ulong() & 0xFFFFFFFF);
        buffer.push_back(word);
        fpga_crc << word;
        link_crc << word;

        // special "header" word from ROC
        word = 0;
        word |= 0b0101 << 28;
        word |= (bunch_id & packing::utility::mask<12>) << 16;
        word |= (rreq & packing::utility::mask<6> ) << 10;
        word |= (orbit & packing::utility::mask<3>) << 7;
        // skipping hamming error bits because we will set them all to false here
        word |= 0b0101;
        buffer.push_back(word);
        fpga_crc << word;
        link_crc << word;
        
        /**
         * TODO: Common-Mode Channel
         *  Somewhere in here is where the common-mode channel
         *  would be inserted. I'm not sure if we should expect
         *  that the common mode channel also has a sample or it
         *  will be reserved and never should have a sample
         */

        // put samples into buffer
        for (auto const& [channel, sample] : channels) {
          buffer.push_back(sample);
          fpga_crc << sample;
          link_crc << sample;
        }
        buffer.push_back(link_crc.get());
        fpga_crc << link_crc.get();
      }
    }
    i_bx++;
  }

  event.add(output_name_, buffer);

  return;
}  // produce

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalRawEncoder);
