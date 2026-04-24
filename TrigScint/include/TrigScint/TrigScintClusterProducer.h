/**
 * @file TrigScintClusterProducer.h
 * @brief Clustering of trigger scintillator hits
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef TRIGSCINT_TRIGSCINTCLUSTERPRODUCER_H
#define TRIGSCINT_TRIGSCINTCLUSTERPRODUCER_H

// LDMX Framework
#include <map>

#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TrigScintCluster.h"
#include "TrigScint/Event/TrigScintHit.h"

namespace trigscint {

/**
 * @class TrigScintClusterProducer
 * @brief
 */
class TrigScintClusterProducer : public framework::Producer {
 public:
  TrigScintClusterProducer(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

  /**
   * add a hit at index idx to a cluster
   */
  virtual void addHit(uint idx, ldmx::TrigScintHit hit);

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  // collection of clusters produced
  std::vector<ldmx::TrigScintCluster> clusters_;

  // cluster seeding threshold
  double seed_{0.};

  // min threshold for adding a hit to a cluster
  double min_thr_{0.};

  // max number of neighboring hits to combine when forming a cluster
  int max_width_{2};

  //PE-amplitude weighting for centroid values
  bool ampl_weighting_{true};  

  // specific verbosity of this producer
  int verbose_{0};

  // expected arrival time of hits in the pad [ns]
  double pad_time_{0.};

  // maximum allowed delay for hits to be considered for clustering
  double time_tolerance_{0.};

  // input collection (hits)
  std::string input_collection_;

  // output collection (clusters)
  std::string output_collection_;

  // specific pass name to use for track making
  std::string pass_name_{""};

  // vertical bar start index
  int vert_bar_start_idx_{52};

  // cluster channel nb centroid (will be content weighted)
  float centroid_{0.};

  // cluster channel nb horizontal centroid (will be content weighted)
  float centroid_x_{-1};

  // cluster channel nb vertical centroid (will be content weighted)
  float centroid_y_{-1};

  // energy (edep), PE, or sth
  float val_{0.};

  // edep content, only; leave val_ for PE
  float val_e_{0.};
  
  // sum of hit cluster weights
  float sumw_{0.};

  // book keep which channels have already been added to the cluster at hand
  std::vector<unsigned int> v_added_indices_;

  // book keep which channels have already been added to any cluster
  std::vector<unsigned int> v_used_indices_;

  // fraction of cluster energy deposition associated with beam electron sim
  // hits
  float beam_e_{0.};

  // cluster time (energy weighted based on hit time)
  float time_{0.};

  // empty map container
  std::map<int, int> hit_channel_map_;
};

}  // namespace trigscint

#endif /* TRIGSCINT_TRIGSCINTCLUSTERPRODUCER_H */
