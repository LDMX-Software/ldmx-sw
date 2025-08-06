/**
 * @file TrigScintFirmwareTracker.h
 * @brief Tracker made to emulate and stage real firmware, emulates existing
 * ldmx software but has LUT structure.
 * @author Rory O'Dwyer, Stanford University
 */

#ifndef TRIGSCINT_TRIGSCINTFIRMWARETRACKER_H
#define TRIGSCINT_TRIGSCINTFIRMWARETRACKER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TrigScintHit.h"
#include "TrigScint/Event/TrigScintTrack.h"
#include "TrigScint/Firmware/objdef.h"
#include "TrigScint/TrigScintFirmwareTracker.h"

namespace trigscint {

/**
 * @class TrigScintFirmwareTracker
 * @brief
 */
class TrigScintFirmwareTracker : public framework::Producer {
 public:
  TrigScintFirmwareTracker(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

  ldmx::TrigScintTrack makeTrack(Track outTrk);

  /**
   * add a hit at index idx to a cluster
   */

 private:
  // min threshold for adding a hit to a cluster
  double minThr_{0.};

  // specific verbosity of this producer
  int verbose_{0};

  // expected arrival time of hits in the pad [ns]
  double padTime_{0.};

  // maximum allowed delay for hits to be considered for clustering
  double timeTolerance_{0.};

  // output collection (clusters)
  std::string output_collection_;

  // input collection (hits_)
  std::string digis1_collection_;
  std::string digis2_collection_;
  std::string digis3_collection_;

  std::vector<ldmx::TrigScintTrack> tracks_;

  // specific pass name to use for track making
  std::string passName_{""};

  // book keep which channels have already been added to the cluster at hand
  std::vector<unsigned int> v_addedIndices_;

  // book keep which channels have already been added to any cluster
  std::vector<unsigned int> v_usedIndices_;

  // empty map container
  std::map<int, int> hitChannelMap_;
};

}  // namespace trigscint

#endif /* TRIGSCINT_TRIGSCINTFIRMWARETRACKER_H */
