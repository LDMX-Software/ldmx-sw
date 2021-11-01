/*
  author: pf
  email: pbutti@slac.stanford.edu
*/


#ifndef TRACKING_SIM_HITSMEARINGPROCESSOR_H_
#define TRACKING_SIM_HITSMEARINGPROCESSOR_H_

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
//#include "Framework/Event.h" //needed??!
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"


/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/Event/SimTrackerHit.h"


/*~~~~~~~~~~~~~*/
/*   stdlib    */
/*~~~~~~~~~~~~~*/

#include <iostream>
#include <random>

namespace tracking {
namespace sim {

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
  std::shared_ptr<std::normal_distribution<float> > distN;  
  std::vector<std::string> m_inputHitCollections;
  std::vector<std::string> m_outputHitCollections;
  
  /* smearing sigmas */
  double m_taggerSigma_u, m_taggerSigma_v;
  double m_recoilSigma_u, m_recoilSigma_v;
     
  ldmx::SimTrackerHit smearSimHit(const ldmx::SimTrackerHit& hit);

  /* random engine */
  std::default_random_engine m_generator;
    
}; // HitSmearingProcessor
    

} // namespace sim
} // namespace tracking

#endif // TRACKING_SIM_HITSMEARINGPROCESSOR_H_
