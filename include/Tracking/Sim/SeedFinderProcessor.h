#ifndef TRACKING_SIM_SEEDFINDERPROCESSOR_H_
#define TRACKING_SIM_SEEDFINDERPROCESSOR_H_


//---< Framework >---//
#include "Framework/Event.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---< Tracking >---//
#include "Tracking/Sim/LdmxSpacePoint.h"
#include "Tracking/Sim/SeedToTrackParamMaker.h"
#include "Tracking/Sim/TrackingUtils.h"

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
      
     private:
      Acts::SpacePointGridConfig grid_conf_;
      Acts::SeedfinderConfig<ldmx::LdmxSpacePoint> config_;
      Acts::SeedFilterConfig seed_filter_cfg_;
      Acts::Vector3 bField_;

      //Acts::Seedfinder::State state_;
      
      std::shared_ptr<Acts::Seedfinder<ldmx::LdmxSpacePoint> > seed_finder_;
      std::shared_ptr<Acts::BinFinder<ldmx::LdmxSpacePoint> >  bottom_bin_finder_;
      std::shared_ptr<Acts::BinFinder<ldmx::LdmxSpacePoint> >  top_bin_finder_;

      /* This is a temporary (working) solution to estimate the track parameters out of the seeds
       * Eventually we should move to what is in ACTS (I'm not happy with what they did regarding this part atm)
       */

      std::shared_ptr<tracking::sim::SeedToTrackParamMaker> seed_to_track_maker_;
      
      double processing_time_{0.};
      long nevents_{0};
      bool debug_{false};
      std::string out_seed_collection_{"SeedTracks"};
      
    }; // SeedFinderProcessor
    

  } // namespace sim
} // namespace tracking

#endif // TRACKING_SIM_SEEDFINDERPROCESSOR_H_
