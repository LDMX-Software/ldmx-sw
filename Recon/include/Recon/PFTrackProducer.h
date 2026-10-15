/**
 * @file PFTrackProducer.h
 * @brief Track selection for PFlow Reco
 * @author Christian Herwig, Fermilab
 */

#ifndef PFTRACKPRODUCER_H
#define PFTRACKPRODUCER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace recon {

/**
 * @class PFTrackProducer
 * @brief Selects the tracks passed to ParticleFlow
 *
 * Outputs ldmx::Track in both modes. In truth mode, Ecal scoring plane hits
 * are wrapped into tracks with a single AtECAL state. In reco mode, tracks
 * are selected from a reconstructed collection and must have an AtECAL state.
 */
class PFTrackProducer : public framework::Producer {
 public:
  PFTrackProducer(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

 private:
  void produceTruth(framework::Event& event);
  void produceReco(framework::Event& event);

  // use Ecal scoring plane hits instead of reconstructed tracks
  bool truth_tracking_{false};

  // name of collection for track inputs to be passed
  std::string input_track_coll_name_;
  // pass name for the input collection
  std::string input_pass_name_;
  // name of collection for pfTracks to be output
  std::string output_track_coll_name_;

  // truth mode
  // boolean to cheat to select only electron tracks
  //  in this cheating truth tracker
  bool do_electron_tracking_{};
  // minimum z_ momentum component allowed for beam electron selection
  double min_electron_momentum_z_{};
  // maximum trackID allowed for beam electron selection
  int max_electron_track_id_{};

  // reco mode
  // name and pass of the reconstructed track collection
  std::string reco_track_coll_name_;
  std::string reco_track_pass_name_;
  // minimum number of hits on track
  int min_n_hits_{};
  // maximum chi2/ndf, negative disables the cut
  double max_chi2_ndf_{};
  // minimum momentum at the Ecal face in MeV
  double min_momentum_{};
};
}  // namespace recon

#endif /* PFTRACKPRODUCER_H */
