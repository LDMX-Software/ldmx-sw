/**
 * @file TrackDeDxMassEstimator.h
 * @brief Class that estimates the mass of a particle using tracker dE/dx
 * information
 * @author Danyi Zhang, Tamas Almos Vami (UCSB)
 */

#ifndef RECON_TRACKDEDXMASSESTIMATOR_H_
#define RECON_TRACKDEDXMASSESTIMATOR_H_

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/TrackDeDxMassEstimate.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"

namespace recon {

/**
 * @class TrackDeDxMassEstimator
 * @brief
 */
class TrackDeDxMassEstimator : public framework::Producer {
 public:
  TrackDeDxMassEstimator(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps) override;

  virtual void produce(framework::Event& event) override;

 private:
  // specific verbosity of this producer
  int verbose_{0};

  float fit_res_C_{0.};
  float fit_res_K_{-9999.};

  // name of input track collection
  std::string trackCollection_;
  // name of input measurement collection
  std::string measCollection_{"DigiTaggerSimHits"};

};  // TrackDeDxMassEstimator

}  // namespace recon

#endif  // RECON_TRACKDEDXMASSESTIMATOR_H_
