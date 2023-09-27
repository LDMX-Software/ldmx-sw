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


#include "Tracking/Event/Track.h"
#include "Tracking/Sim/TrackingUtils.h"

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


  /** Method to extrapolate to a target surface given a set of BoundTrackParameters
   *
   @param Bound Track Parameters
   @param The target surface
   @return optional with BoundTrackParameters
  **/
  
  std::optional<Acts::BoundTrackParameters> extrapolate(const Acts::BoundTrackParameters pars,
                                                        const std::shared_ptr<Acts::Surface>& target_surface) {
    
    //Just to make it explicit
    bool boundaryCheck = false;
    auto intersection = target_surface->intersect(gctx_,
                                                  pars.position(gctx_),
                                                  pars.unitDirection(),
                                                  boundaryCheck);
    
    Acts::PropagatorOptions<ActionList, AbortList> pOptions(gctx_, mctx_);
    pOptions.direction = intersection.intersection.pathLength >= 0
                         ? Acts::Direction::Forward
                         : Acts::Direction::Backward;
    
    
    auto result = propagator_.propagate(pars,
                                        *target_surface,
                                        pOptions);
    
    
    if (result.ok())
      return *result->endParameters;
    else
      return std::nullopt;
  }
  
  
  /** Method to extrapolate to a target surface given a track
   * The method computes which track state is closest to the surface to choose which one to use to
   * extrapolate. This method doesn't use a measurement, but whatever first/last track state is defined. 
   @param An Acts::Track
   @param The target surface 
   @return optional containing the bound track parameters.
   **/
  
  template <class track_t>
  std::optional<Acts::BoundTrackParameters> extrapolate(track_t track,
                                                        const std::shared_ptr<Acts::Surface>& target_surface) {
    
    // get first and last track state on surface
    size_t nstates = track.nTrackStates();
    auto& tsc       = track.container().trackStateContainer();
    //auto  ts_first  = tsc.getTrackState(0);
    //auto  ts_last   = tsc.getTrackState(nstates-1);
    auto  outermost = *(track.trackStates().begin());
    auto  begin  = track.trackStates().begin();
    std::advance(begin, track.nTrackStates() - 1 );
    auto  innermost  = *begin;
    
    // I'm checking which track state is closer to the origin of the target surface to decide
    // from where to start the extrapolation to the surface. I use the coordinate along the beam axis.
    
    double first_dis =  std::abs(innermost.referenceSurface().transform(gctx_).translation()(0)
                                 - target_surface->transform(gctx_).translation()(0));
    
    double last_dis =  std::abs(outermost.referenceSurface().transform(gctx_).translation()(0)
                                - target_surface->transform(gctx_).translation()(0));
    

    //This is the track state to use for the extrapolation
    
    const auto& ts = first_dis < last_dis ? innermost : outermost;

    //std::cout<<"Selected track state for extrapolation"<<std::endl;
    
    //Get the BoundTrackStateParameters
    
    const auto& surface  = ts.referenceSurface();
    const auto& smoothed = ts.smoothed();
    bool hasSmoothed = ts.hasSmoothed();
    const auto& filtered = ts.filtered();
    const auto& cov      = ts.smoothedCovariance();

    //std::cout<<"Surface::"<<     surface.transform(gctx_).translation()<<std::endl;
    //std::cout<<"Smoothed::"<<    smoothed.transpose()<<std::endl;
    //std::cout<<"HasSmoothed::"<< hasSmoothed<<std::endl;
    //std::cout<<"Filtered::"<<    filtered.transpose()<<std::endl;
    
    Acts::ActsScalar q = smoothed[Acts::eBoundQOverP] > 0 ? 1 * Acts::UnitConstants::e
                         : -1 * Acts::UnitConstants::e;
    
    Acts::BoundTrackParameters sp(surface.getSharedPtr(),
                                  smoothed,
                                  q,
                                  cov);
    
    return extrapolate(sp,target_surface);
    
  }
  
  
  
  template <class track_t>
  std::optional<Acts::BoundTrackParameters> extrapolateToEcal(track_t track,
                                                              const std::shared_ptr<Acts::Surface>& target_surface) {
    
    
    // get last track state on the track.
    // Now.. I'm taking whatever it is. I'm not checking here if it is a measurement.
    
    size_t nstates = track.nTrackStates();
    auto& tsc     = track.container().trackStateContainer();
    auto begin = track.trackStates().begin();
    auto ts_last = *begin;
    const auto& surface  = (ts_last).referenceSurface();
    const auto& smoothed = (ts_last).smoothed();
    const auto& cov      = (ts_last).smoothedCovariance();

    //Get the BoundTrackStateParameters
    
    Acts::ActsScalar q = smoothed[Acts::eBoundQOverP] > 0 ? 1 * Acts::UnitConstants::e
                         : -1 * Acts::UnitConstants::e;
    
    Acts::BoundTrackParameters state_parameters(surface.getSharedPtr(),
                                                smoothed,
                                                q,
                                                cov);
    
    // One can also use directly the extrapolate method
    
    Acts::PropagatorOptions<ActionList, AbortList> pOptions(gctx_, mctx_);
    pOptions.direction = Acts::Direction::Forward;
    
    auto result = propagator_.propagate(state_parameters,
                                        *target_surface,
                                        pOptions);
    
    
    if (result.ok())
      return *result->endParameters;
    else
      return std::nullopt;
  }


  /** 
  
  /** Create an ldmx::TrackState to the extrapolated position
   @param Acts::Track
   @param extrapolation surface
   @param ldmx::Track::TrackState
   @param TrackStateType
   @return boolean to check if there was a problem in the extrapolation
   *
   */

  template <class track_t>
  bool TrackStateAtSurface(track_t track,
                           const std::shared_ptr<Acts::Surface>& target_surface,
                           ldmx::Track::TrackState& ts,
                           ldmx::TrackStateType type) {

    auto opt_pars = extrapolate(track,target_surface);
    if (opt_pars) {
      
      //Reference point
      Acts::Vector3 surf_loc = target_surface->transform(gctx_).translation();
      ts.refX = surf_loc(0);
      ts.refY = surf_loc(1);
      ts.refZ = surf_loc(2);
      
      //Parameters
      ts.params = tracking::sim::utils::convertActsToLdmxPars((*opt_pars).parameters());

      //Covariance
      const Acts::BoundMatrix& trk_cov = *((*opt_pars).covariance());
      tracking::sim::utils::flatCov(trk_cov,ts.cov);
      
      ts.ts_type = type;
      return true;
    }
    else {
      return false;
    }
  }
  
  
  
 private:
  
  propagator_t propagator_;
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext mctx_;
  
};

}
}
