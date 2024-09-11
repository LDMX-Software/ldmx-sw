/**
 * @file TrackDeDxMassEstimator.h
 * @brief Class that estimates the mass of a particle using tracker dE/dx information
 * @author Danyi Zhang, UCSB
 */

#ifndef RECON_TRACKDEDXMASSESTIMATOR_H_
#define RECON_TRACKDEDXMASSESTIMATOR_H_

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace recon {

/**
 * @class TrackDeDxMassEstimator
 * @brief
 */
class TrackDeDxMassEstimator : public framework::Producer {
 public:
  TrackDeDxMassEstimator(const std::string &name, framework::Process &process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

 private:
  // specific verbosity of this producer
  int verbose_{0};
  
  float fit_res_C_{0.};
  float fit_res_K_{0.};

  // name of input track collection
  std::string trackCollection_;

};  // TrackDeDxMassEstimator

}  // namespace recon

#endif  // RECON_TRACKMASSESTIMATORDEDX_H_
