#ifndef TRACKING_SIM_HITSMEARINGPROCESSOR_H_
#define TRACKING_SIM_HITSMEARINGPROCESSOR_H_

//---< C++ >---//
#include <iostream>
#include <random>

//---< Framework >---//
#include "Framework/Event.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"

namespace tracking {
namespace sim {

/**
 * @brief Class that performs simulated hit smearing in the tracker
 */
class HitSmearingProcessor : public framework::Producer {

public:
  /**
   * Constructor.
   *
   * @param name The name of the instance of this object.
   * @param process The process running this producer.
   */
  HitSmearingProcessor(const std::string &name, framework::Process &process);

  /// Destructor
  ~HitSmearingProcessor();

  /**
   *
   */
  void onProcessStart() final override;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Run the processor
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event);

private:
  ///
  std::shared_ptr<std::normal_distribution<float>> normal_;

  ///
  std::vector<std::string> input_hit_coll_;
  
  /// 
  std::vector<std::string> output_hit_coll_;

  /// smearing sigmas 
  double tagger_sigma_u_, tagger_sigma_v_;
  double recoil_sigma_u_, recoil_sigma_v_;

  /// 
  ldmx::SimTrackerHit smearSimHit(const ldmx::SimTrackerHit &hit);

  /// random engine 
  std::default_random_engine generator_;

}; // HitSmearingProcessor

} // namespace sim
} // namespace tracking

#endif // TRACKING_SIM_HITSMEARINGPROCESSOR_H_
