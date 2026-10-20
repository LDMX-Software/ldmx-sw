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
  void identifyCharged(ldmx::PFCandidate& cand) const;
  static ldmx::CaloCluster mergeClusters(
      const std::vector<ldmx::CaloCluster>& clusters, int root,
      const std::vector<int>& members);
  std::vector<ldmx::CaloCluster> linkHcalClusters(
      const std::vector<ldmx::CaloCluster>& clusters) const;
  double ecalDepth(const ldmx::CaloCluster& cl) const;
  double ecalWidth(const ldmx::CaloCluster& cl, const ldmx::Track& tk) const;
  void absorbHadronShower(ldmx::PFCandidate& cand, const ldmx::Track& tk,
                          const std::vector<ldmx::CaloCluster>& ecal,
                          std::vector<bool>& ecal_used,
                          const std::vector<ldmx::CaloCluster>& hcal,
                          std::vector<bool>& hcal_used) const;
  int linkTrackHcal(const ldmx::Track& tk,
                    const std::vector<ldmx::CaloCluster>& clusters,
                    const std::vector<bool>& used) const;
  void recoverEcal(ldmx::PFCandidate& cand, const ldmx::Track& tk,
                   const std::vector<ldmx::CaloCluster>& clusters,
                   std::vector<bool>& used) const;
  double trackClusterDist(const std::vector<double>& xyz,
                          const std::vector<double>& pxyz,
                          const ldmx::CaloCluster& cl) const;
  std::vector<ldmx::CaloCluster> linkEcalClusters(
      const std::vector<ldmx::CaloCluster>& clusters,
      const std::vector<ldmx::Track>& tracks) const;

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
  // dE/dx estimates made on the input tracks, empty name disables
  std::string input_mass_coll_name_;
  std::string input_mass_passname_;
  // max RMS-normalized track to Ecal cluster distance
  double link_max_dist_{};
  // floor on the cluster RMS used to normalize the link distance [mm]
  double link_min_rms_{};
  // linked Ecal E/p above which a track is an electron
  double electron_ep_min_{};
  // linked Ecal E/p above which the excess becomes a photon
  double excess_ep_{};
  // dE/dx mass squared windows [MeV^2] and their momentum limits [MeV] for
  // the p and K mass hypotheses of charged hadrons
  double proton_min_mass2_{};
  double kaon_min_mass2_{};
  double proton_max_p_{};
  double kaon_max_p_{};
  // low E/p tracks absorb unlinked clusters within this radius [mm]
  // until E/p reaches recovery_stop_ep, 0 disables
  double recovery_radius_{};
  double recovery_stop_ep_{};
  // Hcal to Hcal linking cone seen from the target [deg], 0 disables
  double hcal_merge_angle_{};
  // track to Hcal link cone from the track at the Ecal face [deg], 0 disables
  double hcal_track_angle_{};
  // charged hadrons absorb free Ecal clusters within this radius [mm] and
  // Hcal clusters in their cone while calo E < absorb_ep * p, 0 disables
  double hadron_ecal_radius_{};
  double hadron_absorb_ep_{};
  // neutral candidates below this energy are dropped [MeV]
  double min_neutral_e_{};
  // electrons need an energy-weighted Ecal depth [mm] below max_depth at
  // 1 GeV, growing by depth_slope per doubling of momentum
  double ecal_front_z_{};
  double electron_max_depth_{};
  double electron_depth_slope_{};
  // Ecal to Ecal linking, radius 0 disables [mm]
  double ecal_link_radius_{};
  double ecal_link_ratio_{};
  int ecal_link_min_layers_{};
  // soft clusters kept as photons: min E [MeV], isolation and start z [mm]
  double photon_keep_min_e_{};
  double photon_keep_min_dist_{};
  double photon_keep_max_zstart_{};
  int dedx_min_hits_{};
};
}  // namespace recon

#endif /* PARTICLEFLOW_H */
