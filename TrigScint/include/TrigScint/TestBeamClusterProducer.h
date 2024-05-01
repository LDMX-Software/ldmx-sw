/**
 * @file TestBeamClusterProducer.h
 * @brief Clustering of TS testbeam hits
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef TRIGSCINT_TESTBEAMCLUSTERPRODUCER_H
#define TRIGSCINT_TESTBEAMCLUSTERPRODUCER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TestBeamHit.h"
#include "TrigScint/Event/TrigScintCluster.h"

namespace trigscint {

/**
 * @class TestBeamClusterProducer
 * @brief
 */
class TestBeamClusterProducer : public framework::Producer {
 public:
  TestBeamClusterProducer(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  /**
   * add a hit at index idx to a cluster
   */
  virtual void addHit(uint idx, trigscint::TestBeamHit hit);

  virtual void onProcessStart();

  virtual void onProcessEnd();

 private:
  // collection of clusters produced
  std::vector<ldmx::TrigScintCluster> clusters_;

  // cluster seeding threshold
  double seed_{0.};

  // min threshold for adding a hit to a cluster
  double minThr_{0.};

  // max number of neighboring hits to combine when forming a cluster
  int maxWidth_{2};

  // max channel number (to avoid unused channels)
  int maxChannelID_{11};

  // specific verbosity of this producer
  int verbose_{0};

  // expected arrival time of hits in the pad [ns]
  double padTime_{0.};

  // maximum allowed delay for hits to be considered for clustering
  double timeTolerance_{0.};

  // input collection (hits)
  std::string input_collection_;

  // output collection (clusters)
  std::string output_collection_;

  // specific pass name to use for track making
  std::string passName_{""};

  // cluster channel nb centroid (will be content weighted)
  float centroid_{0.};

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
  // -- could convert this to instead be a "cleanb frac"; fraction of cluster
  // energy coming from clean hits
  float beamE_{0.};

  /// boolean indicating whether we want to apply quality criteria from hit
  /// reconstruction
  bool doCleanHits_{false};

  // cluster time (energy weighted based on hit time)
  float time_{0.};

  // empty map container
  std::map<int, int> hitChannelMap_;
};

}  // namespace trigscint

#endif /* TRIGSCINT_TESTBEAMCLUSTERPRODUCER_H */
