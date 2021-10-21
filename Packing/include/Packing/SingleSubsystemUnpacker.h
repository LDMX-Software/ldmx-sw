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
 * raw data file into a **single** buffer for a downstream processor
 * to decode.
 */
class SingleSubsystemUnpacker : public framework::Producer {
 public:
  /// normal constructor
  SingleSubsystemUnpacker(const std::string& name, framework::Process& p)
    : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~SingleSubsystemUnpacker() {}

  /**
   * Configure the unpacker and open the raw data file for IO
   *
   * @param[in] ps Parameters for configuration
   */
  void configure(framework::config::Parameters& ps) final override;

  /**
   * Actually do the unpacking/decoding.
   *
   * @param[in,out] event Event bus with raw data where we will put the digis
   */
  void produce(framework::Event& event) final override;

  void beforeNewRun(ldmx::RunHeader& rh) final override;

 private:
  /// number of bytes in each event
  int num_bytes_per_event_;
  /// destination object name
  std::string output_name_;
  /// Detector file name
  std::string detector_name_;

 private:
  /// raw data file we are reading
  utility::Reader reader_;
};  // SingleSubsystemUnpacker

}  // namespace packing

#endif  // PACKING_SINGLESUBSYSTEMUNPACKER_H
