#pragma once

#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"

#include <iostream>
#include <iterator>
#include <optional>


using ActionList = Acts::ActionList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;

namespace tracking{
namespace reco{

template <class propagator_t>

class TrackExtrapolatorTool {
  
  
public:
  
  // The geometry context should be already in the propagator options... 
  TrackExtrapolatorTool(propagator_t propagator,
                        const Acts::GeometryContext& gctx,
                        const Acts::MagneticFieldContext& mctx)
      : propagator_(std::move(propagator)) {
    gctx_       = gctx;
    mctx_       = mctx;
  }
  
  
  std::optional<Acts::BoundTrackParameters> extrapolate(const Acts::BoundTrackParameters pars,
                                                        const std::shared_ptr<Acts::Surface>& target_surface);
  


  /** Method to extrapolate to a target surface given a track
   * The method computes which track state is closest to the surface to choose which one to use to
   * extrapolate. This method doesn't use a measurement, but whatever first/last track state is defined. 
   @param An Acts::Track
   @param The target surface 
   @return optional containing the bound track parameters.
   **/


  // TODO:: CHECK IF THE FIRST TRACK STATE IS THE TARGET STATE SURFACE
  
  template <class track_t>
  std::optional<Acts::BoundTrackParameters> extrapolate(track_t track,
                                                        const std::shared_ptr<Acts::Surface>& target_surface);
      
  //template <class track_t>
  //std::optional<Acts::BoundTrackParameters> extrapolateToEcal(track_t track,
  //                                                            const std::shared_ptr<Acts::Surface>& target_surface);

  template <class track_t>
  std::optional<Acts::BoundTrackParameters> extrapolateToEcal(track_t track,
                                                              const std::shared_ptr<Acts::Surface>& target_surface) {
  
  
    // get last track state on the track.
    // Now.. I'm taking whatever it is. I'm not checking here if it is a measurement.

    size_t nstates = track.nTrackStates();
    std::cout<<"ExtrapolateToEcal:: Getting the last meas::"<<std::endl;
    
    /*
    auto tsRange = track.trackStates();

    for (auto ts : tsRange) {
      const auto& surface = (ts).referenceSurface();
      const auto& smoothed = (ts).smoothed();
      const auto& cov = (ts).smoothedCovariance();
      
      std::cout<<"ExtrapolateToEcal:: Last Measurement surface::"<<std::endl;
      std::cout<<surface.transform(gctx_).translation()<<std::endl;
      std::cout<<surface.geometryId()<<std::endl;
      std::cout<<smoothed<<std::endl;
  
    }
    */

    auto& tsc     = track.container().trackStateContainer();
    auto  ts_last = tsc.getTrackState(nstates-1);
    const auto& surface  = (ts_last).referenceSurface();
    const auto& smoothed = (ts_last).smoothed();
    const auto& cov      = (ts_last).smoothedCovariance();
    
    std::cout<<"ExtrapolateToEcal:: Got the last meas::"<<std::endl;
    
    //Get the BoundTrackStateParameters
    
    Acts::ActsScalar q = smoothed[Acts::eBoundQOverP] > 0 ? 1 * Acts::UnitConstants::e
                         : -1 * Acts::UnitConstants::e;
    
    Acts::BoundTrackParameters state_parameters(surface.getSharedPtr(),
                                                smoothed,
                                                q,
                                                cov);
    
    // One can also use directly the extrapolate method
    
    Acts::PropagatorOptions<ActionList, AbortList> pOptions(gctx_, mctx_);
    pOptions.direction = Acts::NavigationDirection::Forward;
    
    auto result = propagator_.propagate(state_parameters,
                                        *target_surface,
                                        pOptions);
    
    
    if (result.ok())
      return *result->endParameters;
    else
      return std::nullopt;
    
    
  }
  
  
 private:
  
  propagator_t propagator_;
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext mctx_;
  
};

}
}
