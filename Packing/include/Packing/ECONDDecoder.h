#ifndef PACKING_ECONDDECODER_H
#define PACKING_ECONDDECODER_H

#include <array>
#include <memory>

#include "Framework/EventProcessor.h"
#include "Packing/MultiSampleECONDEventPacket.h"

namespace packing {

/**
 * decode event packets built by ECON-D chips
 *
 * The ECON-D chip is used to construct the event packets
 * for both the Ecal and Hcal subsystems, so both will share
 * this decoder.
 *
 * Each event on the bus is one frame from SingleSubsystemUnpacker
 * holding all of the samples from a single ECON-D.
 * The channel samples are put into an HgcrocDigiCollection and
 * the rest of the packet data into an ldmx::ECONDEventInfo.
 * Corrupted packets are flagged and skipped, the digis are only
 * filled if all samples in the event are good.
 *
 * The input can also be the subsystem buffer from the EventBuilder.
 * Events without the input buffer get empty outputs flagged with
 * ldmx::ECONDEventInfo::NO_FRAME.
 */
class ECONDDecoder : public framework::Producer {
 public:
  /// normal constructor
  ECONDDecoder(const std::string& name, framework::Process& p)
      : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~ECONDDecoder() = default;

  /**
   * configure the decoder
   *
   * mostly focused on telling it which subsystem we are decoding
   * (to know how to pack the ID numbers correctly), the input
   * and output event objects.
   */
  void configure(framework::config::Parameters& ps) override;

  /**
   * decode the raw binary data into HgcrocDigiCollection that can
   * be used later for reconstruction or analysis
   */
  void produce(framework::Event& event) override;

  /// print a summary of the decoding
  void onProcessEnd() override;

 private:
  /// get the channel ID for the input link and channel
  unsigned int channelID(int i_link, int i_ch) const;

  /// name of event object of raw binary data
  std::string raw_data_name_;
  /// name of pass to get raw binary data from
  std::string raw_data_pass_;
  /// output name of digi collection
  std::string digi_output_name_;
  /// output name of supporting data object
  std::string info_output_name_;
  /**
   * which calorimeter subsystem we are decoding
   * (true == Ecal, false == Hcal)
   */
  bool is_ecal_{false};
  /// number of enabled eRx on the ECON-D
  int n_links_{2};
  /// optical fiber index used in the electronics ID
  int fiber_{0};
  /// translate electronics IDs into detector IDs
  bool translate_eid_{false};
  /// ROC version for the digi collection
  int roc_version_{3};

  /// decoding buffer
  std::unique_ptr<MultiSampleECONDEventPacket> packet_;

  /// number of frames seen
  long n_frames_{0};
  /// number of events without an input frame
  long n_no_frame_{0};
  /// number of frames whose digis were filled
  long n_frames_filled_{0};
  /// number of frames with a size that is not a multiple of 4 bytes
  long n_bad_frame_size_{0};
  /// number of frames with each frame-level flag
  std::array<long, MultiSampleECONDEventPacket::N_CORRUPTION> n_frame_flags_{};
  /// number of packets seen
  long n_packets_{0};
  /// number of packets decoded without corruption and not empty
  long n_packets_decoded_{0};
  /// number of empty packets (payload length 0)
  long n_packets_empty_{0};
  /// number of corrupted packets (skipped)
  long n_packets_corrupt_{0};
  /// number of packets with each corruption flag
  std::array<long, ECONDEventPacket::N_CORRUPTION> n_packet_flags_{};
  /// number of decoded link sub-packets with bad Stat bits
  long n_links_bad_stat_{0};
  /// number of L1A jumps between consecutive samples
  long n_l1a_jumps_{0};
};

}  // namespace packing

#endif  // PACKING_ECONDDECODER_H
