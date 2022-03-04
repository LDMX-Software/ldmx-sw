#ifndef PACKING_UNPACKER_H
#define PACKING_UNPACKER_H

#include <memory>

#include "Framework/EventProcessor.h"

#include "Packing/RawDataFile/File.h"

namespace packing {

/**
 * @class RawIO
 *
 * This producer unpacks the data from the various subsystems
 * into different branches for later decoding by the subsystem
 * modules. We essentially do no work here and are only interfacing
 * with the RawDataFile submodule.
 */
class RawIO : public framework::Producer {
 public:
  /// normal constructor
  RawIO(const std::string& name, framework::Process& p)
    : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~RawIO() {}

  /**
   * Configure the unpacker and open the raw data file for IO
   *
   * @param[in] ps Parameters for configuration
   */
  void configure(framework::config::Parameters& ps) final override;

  /**
   * We are given a non-const reference to a new RunHeader so
   * we can add parameters unpacked from the raw data.
   */
  void beforeNewRun(ldmx::RunHeader& header) final override;

  /**
   * Actually do the unpacking/decoding.
   *
   * @param[in,out] event Event bus with raw data where we will put the digis
   */
  void produce(framework::Event& event) final override;

  /**
   * Close up ROOT file with raw data in it.
   */
  void onProcessEnd() final override;

 private:
  /// raw data file we are reading
  std::unique_ptr<rawdatafile::File> raw_file_;
};  // RawIO

}  // namespace packing

#endif  // PACKING_UNPACKER_H
