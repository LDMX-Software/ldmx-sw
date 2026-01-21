#ifndef PACKING_SINGLESUBSYSTEMUNPACKER_H
#define PACKING_SINGLESUBSYSTEMUNPACKER_H

#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"
#include "Packing/Utility/Reader.h"

namespace packing {

/**
 * @class SingleSubsystemUnpacker
 *
 * This producer unpacks the data from the a **single** subsystem
 * into a **single** buffer for a downstream processor to decode.
 */
class SingleSubsystemUnpacker : public framework::Producer {
 public:
  /// normal constructor
  SingleSubsystemUnpacker(const std::string& name, framework::Process& p)
      : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~SingleSubsystemUnpacker() = default;

  /**
   * Configure the unpacker and open the raw data file for IO
   *
   * @param[in] ps Parameters for configuration
   */
  void configure(framework::config::Parameters& ps) override;

  /**
   * Actually do the unpacking/decoding.
   *
   * @param[in,out] event Event bus with raw data where we will put the digis
   */
  void produce(framework::Event& event) override;

 private:
  /// subsystem ID to filter for
  int subsystem_;
  /**
   * contributor ID to filter for
   *
   * (-1 means ignore this filter)
   */
  int contributor_;
  /// number of frames to skip before sending data
  int frame_offset_;
  /// destination object name
  std::string output_name_;
  /// raw data file we are reading
  utility::Reader reader_;
  /// frame count from beginning of file for frame_offset_
  int frame_count_{0};
};  // SingleSubsystemUnpacker

}  // namespace packing

#endif  // PACKING_SINGLESUBSYSTEMUNPACKER_H
