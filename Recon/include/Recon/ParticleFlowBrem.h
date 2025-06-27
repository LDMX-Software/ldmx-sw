/**
 * @file ParticleFlow.h
 * @brief Simple PFlow algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef PARTICLEFLOWBREM_H
#define PARTICLEFLOWBREM_H

// LDMX Framework
#include "Ecal/Event/EcalCluster.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Hcal/Event/HcalCluster.h"
#include "Recon/Event/CaloCluster.h"
#include "Recon/Event/PFCandidateBrem.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Track.h"
#include "DetDescr/EcalGeometry.h"
//#include "DetDescr/SimSpecialID.h"

#include "TGraph.h"

namespace recon {

/**
 * @class ParticleFlowBrem
 * @brief
 */
class ParticleFlowBrem : public framework::Producer {
 public:
  ParticleFlowBrem(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  typedef std::pair<float, float> XYCoords;

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onProcessEnd();

  void fillCandTrack(ldmx::PFCandidateBrem& cand, const ldmx::Track& tk, int idx);
  void fillCandEMCalo(ldmx::PFCandidateBrem& cand, const ldmx::EcalCluster& em, int idx);
  void fillCandHadCalo(ldmx::PFCandidateBrem& cand, const ldmx::CaloCluster& had);
  void fillCandBremCalo(ldmx::PFCandidateBrem& cand,const ldmx::EcalCluster& em, int idx);

  std::pair<float, float> getTrajectory(
    std::vector<double> momentum, std::vector<float> position, float z) ;

 private:
  TGraph* eCorr_{0};
  TGraph* hCorr_{0};

  // name of collection for PF inputs to be passed
  std::string inputEcalCollName_;
  std::string inputHcalCollName_;
  std::string inputTaggerTrackCollName_;
  std::string inputRecoilTrackCollName_;

  std::string rocFileName_;
  std::vector<std::vector<double>> roc_range_values_;

  double beamEnergyGeV_{0};



  // name of collection for PF outputs
  std::string outputCollName_;
  // configuration
  bool singleParticle_;

  const ldmx::EcalGeometry* geometry_;

};
}  // namespace recon

#endif /* PARTICLEFLOWBREM_H */
