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
  // collection of clusters produced
  std::vector<ldmx::TrigScintHit> digis1_;

  // collection of clusters produced
  std::vector<ldmx::TrigScintHit> digis2_;

  // collection of clusters produced
  std::vector<ldmx::TrigScintHit> digis3_;

  // min threshold for adding a hit to a cluster
  double minThr_{0.};

  // max number of neighboring hits to combine when forming a cluster
  int maxWidth_{2};

  // specific verbosity of this producer
  int verbose_{0};

  // expected arrival time of hits in the pad [ns]
  double padTime_{0.};

  // maximum allowed delay for hits to be considered for clustering
  double timeTolerance_{0.};

  // output collection (clusters)
  std::string output_collection_;

  // input collection (hits)
  std::string digis1_collection_;
  std::string digis2_collection_;
  std::string digis3_collection_;

  std::vector<ldmx::TrigScintTrack> tracks_;

  // specific pass name to use for track making
  std::string passName_{""};

  // vertical bar start index
  int vertBarStartIdx_{52};

  // cluster channel nb centroid (will be content weighted)
  float centroid_{0.};

  // cluster channel nb horizontal centroid (will be content weighted)
  float centroidX_{-1};

  // cluster channel nb vertical centroid (will be content weighted)
  float centroidY_{-1};

  // energy (edep), PE, or sth
  float val_{0.};

  // edep content, only; leave val_ for PE
  float valE_{0.};

  // book keep which channels have already been added to the cluster at hand
  std::vector<unsigned int> v_addedIndices_;

  // book keep which channels have already been added to any cluster
  std::vector<unsigned int> v_usedIndices_;

  // fraction of cluster energy deposition associated with beam electron sim
  // hits
  float beamE_{0.};

  // cluster time (energy weighted based on hit time)
  float time_{0.};

  // empty map container
  std::map<int, int> hitChannelMap_;
};

}  // namespace trigscint

#endif /* TRIGSCINT_TRIGSCINTFIRMWARETRACKER_H */
