/**
 * @file ParticleFlow.h
 * @brief Simple PFlow algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef PARTICLEFLOW_H
#define PARTICLEFLOW_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Recon/Event/CaloCluster.h"
#include "Recon/Event/PFCandidate.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Track.h"

namespace recon {

/**
 * @class ParticleFlow
 * @brief
 */
class ParticleFlow : public framework::Producer {
 public:
  ParticleFlow(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onProcessEnd();

  void fillCandTrack(ldmx::PFCandidate& cand, const ldmx::Track& tk,
                     const std::vector<ldmx::SimTrackerHit>& sp_hits);
  void fillCandEMCalo(ldmx::PFCandidate& cand, const ldmx::CaloCluster& em);
  void fillCandHadCalo(ldmx::PFCandidate& cand, const ldmx::CaloCluster& had);

 private:
  const std::vector<ldmx::CaloCluster> getEcalClusters(
      framework::Event& event, std::string inputClusterCollName,
      std::string inputClusterPassName);

  static double correction(const std::vector<double>& energies,
                           const std::vector<double>& factors, double energy);

  // energy correction tables: raw energy [MeV] and factor
  std::vector<double> ecal_corr_energies_;
  std::vector<double> ecal_corr_factors_;
  std::vector<double> hcal_corr_energies_;
  std::vector<double> hcal_corr_factors_;

  // name of collection for PF inputs to be passed
  std::string input_ecal_coll_name_;
  std::string input_hcal_coll_name_;
  std::string input_track_coll_name_;
  std::string input_ecal_passname_;
  std::string input_hcal_passname_;
  std::string input_tracks_passname_;
  // Ecal scoring plane hits used for candidate truth info
  std::string truth_sp_coll_name_;
  std::string truth_sp_passname_;
  // bool to toggle using pre-existing clusters instead of making new
  bool use_existing_ecal_clusters_;
  // name of collection for PF outputs
  std::string output_coll_name_;
  // configuration
  bool single_particle_;
};
}  // namespace recon

#endif /* PARTICLEFLOW_H */
