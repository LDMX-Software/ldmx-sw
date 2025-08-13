#ifndef DQM_ECALDIGIVERIFIER_H
#define DQM_ECALDIGIVERIFIER_H

// LDMX Framework

#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace dqm {

/**
 * @class EcalDigiVerifier
 * @brief Generate histograms to check digi pipeline performance
 */
class EcalDigiVerifier : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  EcalDigiVerifier(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Input python configuration parameters
   */
  virtual void configure(framework::config::Parameters& ps);

  /**
   * Fills histograms
   */
  virtual void analyze(const framework::Event& event);

 private:
  /// Collection Name for SimHits
  std::string ecal_sim_hit_coll_;

  /// Pass Name for SimHits
  std::string ecal_sim_hit_pass_;

  /// Collection Name for RecHits
  std::string ecal_rec_hit_coll_;

  /// Pass Name for RecHits
  std::string ecal_rec_hit_pass_;

  /// Number of layers in the ECAL
  int num_layers_;
};
}  // namespace dqm

#endif /* DQM_ECALDIGIVERIFIER_H */
