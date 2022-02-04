#ifndef TRACKING_SIM_HITSMEARINGPROCESSOR_H_
#define TRACKING_SIM_HITSMEARINGPROCESSOR_H_

//---< C++ >---//
#include <iostream>
#include <random>

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"

namespace tracking::sim {

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
  ~HitSmearingProcessor() = default;

  /**
   * Callback called at the beginning of a new run.
   */
  void onNewRun(const ldmx::RunHeader &) final override;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Run the processor. In this instance, it will retrieve the specified
   * collection of SimTrackerHits and smear them.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event) final override;

private:
  /**
   * Smear all hits in a collection. This will create a copy of the input collection and pass
   * each hit to HitSmearingProcessor::smearHit for smearing.
   * 
   * @param[in] hits SimTrackerHit collection to smear
   * @return[out] Smeared hit collection
   */
  std::vector<ldmx::SimTrackerHit>
  smearHits(const std::vector<ldmx::SimTrackerHit> &hits);

  /**
   * Smear a SimTrackerHit according to two independent Gaussian distributions
   * in the u and v direction.
   *
   * @param[in] hit The SimTrackerHit to smear.
   */
  void smearHit(ldmx::SimTrackerHit &hit);

  ///
  std::shared_ptr<std::normal_distribution<float>> normal_;

  /// The input SimTrackerHit collection to smear
  std::string input_hit_coll_;

  /// The smeared SimTrackerHit collection
  std::string output_hit_coll_;

  /// u-direction sigma
  double sigma_u_{0};

  /// v-direction sigma
  double sigma_v_{0};

  /// random engine
  std::default_random_engine generator_;

}; // HitSmearingProcessor

} // namespace tracking::sim

#endif // TRACKING_SIM_HITSMEARINGPROCESSOR_H_
