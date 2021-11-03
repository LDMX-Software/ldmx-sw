#ifndef TRACKING_SIM_SEEDFINDERPROCESSOR_H_
#define TRACKING_SIM_SEEDFINDERPROCESSOR_H_


//---< Framework >---//
#include "Framework/Event.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---< Tracking >---//
#include "Tracking/Sim/LdmxSpacePoint.h"

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"

//---< STD C++ >---//

#include <iostream>

//---< ACTS >---//

#include "Acts/Seeding/SpacePointGrid.hpp"
#include "Acts/Seeding/Seedfinder.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/Seed.hpp"
#include "Acts/Seeding/BinFinder.hpp"
#include "Acts/Seeding/BinnedSPGroup.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"


namespace tracking {
  namespace sim {

    class SeedFinderProcessor : public framework::Producer {

    public:
      /**
       * Constructor.
       *
       * @param name The name of the instance of this object.
       * @param process The process running this producer.
       */
      SeedFinderProcessor(const std::string &name, framework::Process &process);

      /// Destructor
      ~SeedFinderProcessor();

      /**
       *
       */
      void onProcessStart() final override;

      /**
       *
       */
      void onProcessEnd() final override;
       

      /**
       * Configure the processor using the given user specified parameters.
       *
       * @param parameters Set of parameters used to configure this processor.
       */
      void configure(framework::config::Parameters &parameters) final override;

      /**
       * Run the processor and create a collection of results which
       * indicate if a charge particle can be found by the recoil tracker.
       *
       * @param event The event to process.
       */
      void produce(framework::Event &event);

      ldmx::LdmxSpacePoint* convertSimHitToLdmxSpacePoint(const ldmx::SimTrackerHit& hit);
      
    private:
      Acts::SpacePointGridConfig gridConf_;
      Acts::SeedfinderConfig<ldmx::LdmxSpacePoint> m_config;
      Acts::SeedFilterConfig m_seedFilter_cfg;

      //Acts::Seedfinder::State state_;
      
      std::shared_ptr<Acts::Seedfinder<ldmx::LdmxSpacePoint> > seed_finder_;
      std::shared_ptr<Acts::BinFinder<ldmx::LdmxSpacePoint> >  bottom_bin_finder_;
      std::shared_ptr<Acts::BinFinder<ldmx::LdmxSpacePoint> >  top_bin_finder_;  
      
      double m_processingTime{0.};
      long m_nevents{0};
      
    }; // SeedFinderProcessor
    

  } // namespace sim
} // namespace tracking

#endif // TRACKING_SIM_SEEDFINDERPROCESSOR_H_
