#pragma once

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tracking/Reco/TrackingGeometryUser.h"

//--- EDM ---//
#include "Tracking/Event/Measurement.h"
#include "Ecal/Event/EcalHit.h"
#include "Hcal/Event/HcalHit.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TrackExtrapolatorTool.h"

//--- Propagator writer ---//
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
//--- Interpolated magnetic field ---//
#include "Tracking/Sim/BFieldXYZUtils.h"


#include "Tracking/Sim/PropagatorStepWriter.h"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "EventDisplay/json/include/nlohmann/json.hpp"
#include <memory>

using json = nlohmann::json;
using ojson = nlohmann::ordered_json;
using ActionList = Acts::ActionList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;

namespace tracking {
  namespace reco {
  
    class PhoenixEventDumper final : public TrackingGeometryUser {

   public:

      PhoenixEventDumper(const std::string &name, framework::Process &process);
      ~PhoenixEventDumper() = default;


      void configure(framework::config::Parameters &parameters) final override;
      void onProcessStart() override {};
      
      /**
       * onNewRun is the first function called for each processor
       * *after* the conditions are fully configured and accessible.
       * This is where you could create single-processors, multi-event
       * calculation objects.
       */
      void onNewRun(const ldmx::RunHeader& rh) override;
      
      void produce(framework::Event &event) final override;

      
      json prepareTrack(framework::Event& event,
                        const ldmx::Track& track,
                        const std::vector<ldmx::Measurement> measurements,
                        const geo::TrackersTrackingGeometry&tg );
      
   private:

      std::string outfilename_;

      std::string TaggerTracks_{}; 
      std::string RecoilTracks_{};

      std::string TaggerMeasurements_{};
      std::string RecoilMeasurements_{};

      std::string EcalHits_{};
      std::string HcalHits_{};
      
      int eventnr_{0};
      int nevents_{0};


      //--- Track propagation Writer ---//
      std::unique_ptr<tracking::sim::PropagatorStepWriter> prop_writer_;
      

      //--- Extrapolator ---//
      using CkfPropagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;

      std::unique_ptr<CkfPropagator> propagator_;
      
      std::shared_ptr<tracking::reco::TrackExtrapolatorTool<CkfPropagator>> trk_extrap_ ;
      
      //--- BField ---//
      std::string field_map_{""};
      
      //--- Event Display ---//
      
      json evtjson_;
      
    };
    
  }
}
