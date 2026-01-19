#pragma once
#ifndef PACKING_ROGUEUNPACKER_H
#define PACKING_ROGUEUNPACKER_H

#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"
#include "Packing/Utility/Reader.h"

namespace packing {

/**
 * @class RogueUnpacker
 *
 * This producer unpacks the data from the a file written by Rogue's
 * StreamWriter into a set of branches of raw data separated by
 * subsystem.
 */
class RogueUnpacker : public framework::Producer {
 public:
  /// normal constructor
  RogueUnpacker(const std::string& name, framework::Process& p)
      : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~RogueUnpacker() = default;

  /**
   * Configure the unpacker and open the raw data file for IO
   *
   * @param[in] ps Parameters for configuration
   */
  void configure(framework::config::Parameters& ps) override;

  /**
   * Actually do the unpacking
   *
   * @param[in,out] event Event bus where raw data will be placed
   */
  void produce(framework::Event& event) override;

  void beforeNewRun(ldmx::RunHeader& rh) override;

 private:
  /// subsystem IDs to unpack for each event
  std::vector<int> subsystems_;
  /// corresponding contributor ID (-1 if should be ignored)
  std::vector<int> contribs_;
  /// corresponding output name for each subsystem
  std::vector<std::string> subsystem_name_;
  /// Detector file name
  std::string detector_name_;

 private:
  /// raw data file we are reading
  utility::Reader reader_;
};  // RogueUnpacker

}  // namespace packing

#endif  // PACKING_SINGLESUBSYSTEMUNPACKER_H
