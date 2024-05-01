#ifndef HCALRAWDECODER_H
#define HCALRAWDECODER_H

#include <bitset>
#include <iomanip>
#include <optional>

#include "DetDescr/HcalElectronicsID.h"
#include "DetDescr/HcalID.h"
#include "Framework/EventProcessor.h"
#include "Hcal/HcalDetectorMap.h"
#include "Packing/Utility/CRC.h"
#include "Packing/Utility/Mask.h"
#include "Packing/Utility/Reader.h"
#include "Recon/Event/HgcrocDigiCollection.h"
namespace hcal {

struct PolarfireEventHeader {
  /// version of daq format
  int version;
  /// id for polarfire
  int fpga;
  /// number of samples
  int nsamples;
  /// spill number
  int spill;
  /// number of 5MHz ticks since spill
  int ticks;
  /// bunch number according to this polarfire
  int bunch;
  /// event number according to this polarfire
  int number;
  /// run number according to this polarfire
  int run;
  /// day of month run started
  int DD;
  /// month run started
  int MM;
  /// hour run started
  int hh;
  /// minute run started
  int mm;
  /// quality of link headers
  std::vector<bool> good_bxheader;
  /// quality of link trailers
  std::vector<bool> good_trailer;

  /**
   * board us onto the bus with the input prefix
   */
  void board(framework::Event& event, const std::string& prefix);
};

/**
 * @class HcalRawDecoder
 */
class HcalRawDecoder : public framework::Producer {
 public:
  HcalRawDecoder(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}
  virtual ~HcalRawDecoder() = default;
  void configure(framework::config::Parameters&) override;
  /// add detector name if we are reading from file
  void beforeNewRun(ldmx::RunHeader& rh) override;
  /// use read function to decode data, then translate EIDs into DetIDs
  void produce(framework::Event& event) override;

 private:
  /**
   * Assume input reader behaves like a binary data input stream
   * where we can "pop" individual 32-bit words with operator>>
   * and we can check if the reader is done using operator bool()
   *
   * Note: Contains quite a lot of debug details and variables that aren't
   * actually used but should be kept around. Could possibly be cleaned up at
   * some point but for now, best to leave alone
   */
  template <typename ReaderType>
  std::map<ldmx::HcalElectronicsID,
           std::vector<ldmx::HgcrocDigiCollection::Sample>>
  read(ReaderType& reader, PolarfireEventHeader& eh) {
    /**
     * Static parameters depending on ROC version
     */
    static const unsigned int common_mode_channel = roc_version_ == 2 ? 19 : 1;
    static const unsigned int calib_channel = 20;
    /// words for reading and decoding
    static uint32_t head1, head2, w;

    // special header words not counted in event length
    do {
      reader >> head1;
    } while (head1 != 0xbeef2021 and head1 != 0xbeef2022);

    /**
     * Decode event header
     */
    long int eventlen;
    long int i_event{0};
    /* whole event header word looks like
     *
     * VERSION (4) | FPGA ID (8) | NSAMPLES (4) | LEN (16)
     */
    reader >> head1;
    i_event++;

    eh.version = (head1 >> 28) & packing::utility::mask<4>;
    eh.fpga = (head1 >> 20) & packing::utility::mask<8>;
    eh.nsamples = (head1 >> 16) & packing::utility::mask<4>;
    eventlen = head1 & packing::utility::mask<16>;
    if (eh.version == 1u) {
      // eventlen is 32-bit words in event
      // do nothing here
    } else if (eh.version == 2u) {
      // eventlen is 64-bit words in event,
      // need to multiply by 2 to get actual 32-bit event length
      eventlen *= 2;
      // and subtract off the special header word above
      eventlen -= 1;
    } else {
      EXCEPTION_RAISE(
          "VersMis",
          "HcalRawDecoder only knows version 1 and 2 of DAQ format.");
    }
    // sample counters
    int n_words{0};
    std::vector<uint32_t> length_per_sample(eh.nsamples, 0);
    for (uint32_t i_sample{0}; i_sample < eh.nsamples; i_sample++) {
      if (i_sample % 2 == 0) {
        n_words++;
        reader >> w;
        i_event++;
      }
      uint32_t shift_in_word = 16 * (i_sample % 2);
      length_per_sample[i_sample] =
          (w >> shift_in_word) & packing::utility::mask<12>;
    }

    if (eh.version == 2) {
      /**
       * For the time being, the number of sample lengths is fixed to make the
       * firmware for DMA readout simpler. This means we readout the leftover
       * dummy words to move the pointer on the reader.
       */
      for (int i_word{n_words}; i_word < 8; i_word++) {
        reader >> head1;
        i_event++;
      }

      /**
       * extended event header in version 2
       */
      reader >> head1;
      i_event++;
      eh.spill = ((head1 >> 12) & 0xfff);
      eh.bunch = (head1 & 0xfff);
      reader >> head1;
      i_event++;
      eh.ticks = head1;
      reader >> head1;
      i_event++;
      eh.number = head1;
      reader >> head1;
      i_event++;
      eh.run = (head1 & 0xFFF);
      eh.DD = (head1 >> 23) & 0x1F;
      eh.MM = (head1 >> 28) & 0xF;
      eh.hh = (head1 >> 18) & 0x1F;
      eh.mm = (head1 >> 12) & 0x3F;
    }

    /**
     * Re-sort the data from grouped by bunch to by channel
     *
     * The readout chip streams the data off of it, so it doesn't
     * have time to re-group the signals across multiple bunches (samples)
     * by their channel ID. We need to do that here.
     */
    // fill map of **electronic** IDs to the digis that were read out
    std::map<ldmx::HcalElectronicsID,
             std::vector<ldmx::HgcrocDigiCollection::Sample>>
        eid_to_samples;
    std::size_t i_sample{0};
    while (i_event < eventlen) {
      reader >> head1 >> head2;
      i_event += 2;
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

      [[maybe_unused]] uint32_t hgcroc_version =
          (head1 >> 28) & packing::utility::mask<4>;
      uint32_t fpga = (head1 >> 20) & packing::utility::mask<8>;
      uint32_t nlinks = (head1 >> 14) & packing::utility::mask<6>;
      [[maybe_unused]] uint32_t len = head1 & packing::utility::mask<12>;

      fpga_crc << head2;

      [[maybe_unused]] uint32_t bx_id =
          (head2 >> 20) & packing::utility::mask<12>;
      [[maybe_unused]] uint32_t rreq =
          (head2 >> 10) & packing::utility::mask<10>;
      [[maybe_unused]] uint32_t orbit = head2 & packing::utility::mask<10>;

      std::vector<uint32_t> length_per_link(nlinks, 0);
      for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
        if (i_link % 4 == 0) {
          i_event++;
          reader >> w;
          fpga_crc << w;
        }
        uint32_t shift_in_word = 8 * (i_link % 4);
        [[maybe_unused]] bool rid_ok =
            ((w >> (shift_in_word + 7)) & packing::utility::mask<1>) == 1;
        [[maybe_unused]] bool cdc_ok =
            ((w >> (shift_in_word + 6)) & packing::utility::mask<1>) == 1;
        length_per_link[i_link] =
            (w >> shift_in_word) & packing::utility::mask<6>;
      }

      /** Decode Each Link in Sequence
       * Now we should be decoding each link serially
       * where each link was encoded as in Table 4 of
       * the DAQ specs
       *
       * ROC_ID (16) | CRC ok (1) | 0 (7) | RO Map (8)
       * RO Map (32)
       */
      eh.good_bxheader.resize(nlinks);
      eh.good_trailer.resize(nlinks);
      for (uint32_t i_link{0}; i_link < nlinks; i_link++) {
        /**
         * If minimum length of 2 is not written for this link,
         * assume it went down and skip
         */
        if (length_per_link.at(i_link) < 2) {
          continue;
        }
        // move on from last word counting links or previous link
        packing::utility::CRC link_crc;
        i_event++;
        reader >> w;
        fpga_crc << w;
        link_crc << w;
        [[maybe_unused]] uint32_t roc_id =
            (w >> 16) & packing::utility::mask<16>;
        [[maybe_unused]] bool crc_ok =
            ((w >> 15) & packing::utility::mask<1>) == 1;

        // get readout map from the last 8 bits of this word
        // and the entire next word
        std::bitset<40> ro_map = w & packing::utility::mask<8>;
        ro_map <<= 32;
        i_event++;
        reader >> w;
        fpga_crc << w;
        link_crc << w;
        ro_map |= w;
        // loop through channels on this link,
        //  since some channels may have been suppressed because of low
        //  amplitude the channel ID is not the same as the index it
        //  is listed in.
        int j{-1};
        for (uint32_t i_word{2}; i_word < length_per_link.at(i_link);
             i_word++) {
          // skip zero-suppressed channel IDs
          do {
            j++;
          } while (j < 40 and not ro_map.test(j));

          // next word is this channel
          i_event++;
          reader >> w;
          fpga_crc << w;

          if (j == 0) {
            /** Special "Header" Word from ROC
             *
             * version 3:
             * 0101 | BXID (12) | RREQ (6) | OR (3) | HE (3) | 0101
             *
             * version 2:
             * 10101010 | BXID (12) | WADD (9) | 1010
             */
            link_crc << w;
            // v2
            eh.good_bxheader[i_link] = ((w & 0xff000000) == 0xaa000000);
            // v3
            //
            // Shadows a previous declaration but is never used so renamed here
            [[maybe_unused]] uint32_t v3_bx_id =
                (w >> 16) & packing::utility::mask<12>;
            [[maybe_unused]] uint32_t short_event =
                (w >> 10) & packing::utility::mask<6>;
            [[maybe_unused]] uint32_t short_orbit =
                (w >> 7) & packing::utility::mask<3>;
            [[maybe_unused]] uint32_t hamming_errs =
                (w >> 4) & packing::utility::mask<3>;
          } else if (j == common_mode_channel) {
            /** Common Mode Channels
             * 10 | 0000000000 | Common Mode ADC 0 (10) | Common Mode ADC 1 (10)
             */
            link_crc << w;
          } else if (j == calib_channel) {
            // calib channel
            link_crc << w;
          } else if (j == 39) {
            // trailer on each link added by ROC
            // ROC v2 - IDLE word
            // ROC v3 - CRC checksum
            if (roc_version_ == 2) {
              bool good_idle = (w == 0xaccccccc);
              eh.good_trailer[i_link] = good_idle;
            } else {
              bool good_crc = (link_crc.get() == w);
              eh.good_trailer[i_link] = good_crc;
            }
            /*
            if (roc_version_ > 2 and link_crc.get() != w) {
              EXCEPTION_RAISE("BadCRC",
                              "Our calculated link checksum doesn't match the "
                              "one from raw data.");
            }
            */
          } else {
            /// DAQ Channels

            link_crc << w;
            /**
             * The HGC ROC has some odd behavior in terms of reading out the
             * different channels.
             * - extra header word in row j = 0
             * - common mode channel in row (j) number 19 or 1 (depending on the
             * version)
             * - calib channel in row j = 20
             * This introduces a special shift for the channel number to "align"
             * with the range 0-35 per link.
             *
             *  polarfire fpga = fpga readout
             *  roc = i_link / 2 // integer division
             *  channel = j - 1 - (j > common_mode_channel)*1 - (j >
             * calib_channel)*1
             */
            ldmx::HcalElectronicsID eid(fpga, i_link,
                                        j - 1 - 1 * (j > common_mode_channel) -
                                            1 * (j > calib_channel));
            // copy data into EID->sample map
            eid_to_samples[eid].emplace_back(w);
          }  // type of channel
        }    // loop over channels (j in Table 4)
      }      // loop over links

      // another CRC checksum from FPGA
      i_event++;
      reader >> w;
      [[maybe_unused]] uint32_t crc = w;
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
      // padding to reach 64-bit boundary in version 2
      if (eh.version == 2u and length_per_sample.at(i_sample) % 2 == 1) {
        i_event++;
        reader >> head1;
      }
      i_sample++;
    }

    if (eh.version == 1u) {
      // special footer words
      reader >> head1 >> head2;
    }

    return eid_to_samples;
  }

 private:
  /// input file of encoded data
  std::string input_file_;
  /// input object of encoded data
  std::vector<std::string> input_names_;
  /// input pass of creating encoded data
  std::string input_pass_;
  /// output object to put onto event bus
  std::string output_name_;
  /// the detector name if we are reading from a file
  std::string detector_name_;
  /// version of HGC ROC we are decoding
  int roc_version_;
  /// are get translating electronic IDs?
  bool translate_eid_;
  /// is the input_name a file or an event object
  bool read_from_file_;

 private:
  /// the file reader (if we are doing that)
  packing::utility::Reader file_reader_;
};
}  // namespace hcal
#endif /* HCALRAWDECODER_H */
