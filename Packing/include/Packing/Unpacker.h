#ifndef PACKING_UNPACKER_H
#define PACKING_UNPACKER_H

#include "Framework/EventProcessor.h"

#include "Packing/RawDataFile/Reader.h"

namespace packing {

/**
 * @class Unpacker
 *
 * This producer unpacks the data from the various subsystems
 * into different branches for later decoding by the subsystem
 * modules.
 */
class Unpacker : public framework::Producer {
 public:
  /// normal constructor
  Unpacker(const std::string& name, framework::Process& p)
    : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~Unpacker() {}

  /**
   * Configure the unpacker
   *
   * @param[in] ps Parameters for configuration
   */
  void configure(framework::config::Parameters& ps) final override;

  /**
   * Open up raw binary file.
   */
  void onProcessStart() final override;

  /**
   * We are given a non-const reference to a new RunHeader so
   * we can add parameters unpacked from the raw data.
   *
   * This is where we open up the run tree and get the run parameters.
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
  /// name of ROOT file with raw data in it
  std::string raw_file_;
  /// Should we skip branches we can't find translators for?
  bool skip_unavailable_;

 private:
  /// Reader for our raw binary file
  std::unique_ptr<rawdatafile::Reader> reader_;
};  // Unpacker

}  // namespace packing

#endif  // PACKING_UNPACKER_H
