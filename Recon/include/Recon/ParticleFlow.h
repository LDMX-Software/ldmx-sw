/**
 * @file ParticleFlow.h
 * @brief Simple PFlow algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef PARTICLEFLOW_H
#define PARTICLEFLOW_H

// LDMX Framework
#include "Ecal/Event/EcalCluster.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Hcal/Event/HcalCluster.h"
#include "Recon/Event/CaloCluster.h"
#include "Recon/Event/PFCandidate.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "TGraph.h"

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

  void fillCandTrack(ldmx::PFCandidate& cand, const ldmx::SimTrackerHit& tk);
  void fillCandCalo(ldmx::PFCandidate& cand, const ldmx::CaloCluster& cl, TGraph gResponse,int PIDnb);
  void fillCandEMCalo(ldmx::PFCandidate& cand, const ldmx::CaloCluster& em);
  void fillCandHadCalo(ldmx::PFCandidate& cand, const ldmx::CaloCluster& had);

 private:
  const std::vector<ldmx::CaloCluster> getEcalClusters(
      framework::Event& event, std::string inputClusterCollName,
      std::string inputClusterPassName);

  TGraph* eCorr_{0};
  TGraph* hCorr_{0};

  // name of collection for PF inputs to be passed
  std::string inputEcalCollName_;
  std::string inputHcalCollName_;
  std::string inputTrackCollName_;
  std::string input_ecal_passname_;
  std::string input_hcal_passname_;
  std::string input_tracks_passname_;
  // bool to toggle using pre-existing clusters instead of making new
  bool use_existing_ecal_clusters_;
  // name of collection for PF outputs
  std::string outputCollName_;
  // configuration
  bool singleParticle_;
};
}  // namespace recon

#endif /* PARTICLEFLOW_H */
