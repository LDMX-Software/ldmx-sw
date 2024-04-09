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
#include "TGraph.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Recon/Event/CaloCluster.h"
#include "Ecal/Event/EcalCluster.h"
#include "Hcal/Event/HcalCluster.h"
#include "Recon/Event/PFCandidate.h"

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

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

  void fillCandTrack(ldmx::PFCandidate &cand, const ldmx::SimTrackerHit &tk);
  void fillCandEMCalo(ldmx::PFCandidate &cand, const ldmx::CaloCluster &em);
  void fillCandHadCalo(ldmx::PFCandidate &cand, const ldmx::CaloCluster &had);

 private:
  // specific verbosity of this producer
  int verbose_{0};

  TGraph* eCorr_{0};
  TGraph* hCorr_{0};

  // name of collection for PF inputs to be passed
  std::string inputEcalCollName_;
  std::string inputHcalCollName_;
  std::string inputTrackCollName_;
  // name of collection for PF outputs
  std::string outputCollName_;
  // configuration
  bool singleParticle_;
};
}  // namespace recon

#endif /* PARTICLEFLOW_H */
