#include "Tracking/Reco/TrackExtrapolatorTool.h"

namespace tracking {
namespace reco {

template <class propagator_t>
std::optional<Acts::BoundTrackParameters> TrackExtrapolatorTool<propagator_t>::extrapolate(const Acts::BoundTrackParameters pars,
                                                                             const std::shared_ptr<Acts::Surface>& target_surface) {

  //Just to make it explicit
  bool boundaryCheck = false;
  auto intersection = target_surface->intersect(gctx_,
                                                pars.position(gctx_),
                                                pars.unitDirection(),
                                                boundaryCheck);
    
  Acts::PropagatorOptions<ActionList, AbortList> pOptions(gctx_, mctx_);
  pOptions.direction = intersection.intersection.pathLength >= 0
                       ? Acts::NavigationDirection::Forward
                       : Acts::NavigationDirection::Backward;
    
    
  auto result = propagator_.propagate(pars,
                                      *target_surface,
                                      pOptions);
    
    
  if (result.ok())
    return *result->endParameters;
  else
    return std::nullopt;
}

template <class propagator_t>
template <class track_t>
std::optional<Acts::BoundTrackParameters> TrackExtrapolatorTool<propagator_t>::extrapolate(track_t track,
                                                                                           const std::shared_ptr<Acts::Surface>& target_surface) {
    
  // get first and last track state on surface
  auto first_ts = *(track.trackStates().begin());
  auto last_ts  = *(std::prev(track.trackStates().end()));
    
  // I'm checking which track state is closer to the origin of the surface to decide
  // from where to start the extrapolation to the surface. I use the coordinate along the beam axis.
    
  double first_dis =  first_ts.referenceSurface().transform(gctx_).translatrion()(0)
                      - target_surface->transform(gctx_).translation()(0);
    
  double last_dis =  last_ts.referenceSurface().transform(gctx_).translatrion()(0)
                     - target_surface->transform(gctx_).translation()(0);
    
  const auto& ts = first_ts < last_ts ? first_ts : last_ts;
    
  //Get the BoundTrackStateParameters
    
  const auto& surface  = ts.referenceSurface();
  const auto& smoothed = ts.smoothed();
  const auto& cov      = ts.smoothedCovariance();
    
  Acts::ActsScalar q = smoothed[Acts::eBoundQOverP] > 0 ? 1 * Acts::UnitConstants::e
                       : -1 * Acts::UnitConstants::e;
    
  Acts::BoundTrackParameters sp(surface.getSharedPtr(),
                                smoothed,
                                q,
                                cov);
    
  return extrapolate(sp,target_surface);
    
}
/*   
template <class propagator_t>
template <class track_t>
std::optional<Acts::BoundTrackParameters> TrackExtrapolatorTool<propagator_t>::extrapolateToEcal(track_t track,
                                                                                                 const std::shared_ptr<Acts::Surface>& target_surface) {
  
  
  // get last track state on the track.
  // Now.. I'm taking whatever it is. I'm not checking here if it is a measurement.
    
  auto ts = *(std::prev(track.trackStates().end()));
    
  //Alternatively check that it's a measurement with a find_if.

  //auto ts = std::find_if(track.trackStates().end(),
  //track.trackStates().begin(),
  //[&](const auto& state) {
  // auto typeFlags = state.typeFlags();
  // if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag))
  // return true;
  //});
    
  //Get the BoundTrackStateParameters
    
  const auto& surface = ts.referenceSurface();
  const auto& smoothed = ts.smoothed();
  const auto& cov = ts.smoothedCovariance();

  std::cout<<"ExtrapolateToEcal:: Last Measurement surface::"<<std::endl;
  std::cout<<surface.transform(gctx_).translation()<<std::endl;
  std::cout<<surface.geometryId()<<std::endl;
  std::cout<<smoothed<<std::endl;

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
    
    
  
    
  //size_t ts_size = track.terackStates().size();
  for (auto its = track.trackStates().end(); its != track.trackStates().begin(); ++its) {
  auto ts        = *its;
  auto typeFlags = ts.typeFlags();
      
  if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {

  //Get the surface
  const auto& surface = ts.referenceSurface();
        
  //Get the smoothed track params
  Acts::BoundVector smoothed = ts.smoothed();
        
  
        
  break;
  }
  }
    
  } */

} //namespace reco
} //namespace tracking
