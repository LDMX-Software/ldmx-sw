#pragma once

#include <iostream>
#include <iterator>
#include <optional>

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/ParticleHypothesis.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Tracking/Event/Track.h"
#include "Tracking/Sim/TrackingUtils.h"

using ActionList = Acts::ActorList<Acts::detail::SteppingLogger,
                                   Acts::MaterialInteractor,
                                   Acts::EndOfWorldReached>;

namespace tracking {
namespace reco {

template <class propagator_t>
class TrackExtrapolatorTool {
 public:
  // The geometry context should be already in the propagator options...
  TrackExtrapolatorTool(propagator_t propagator,
                        const Acts::GeometryContext& gctx,
                        const Acts::MagneticFieldContext& mctx)
      : propagator_(std::move(propagator)), gctx_(gctx), mctx_(mctx) {}

  /**
   * Turn on/off internal debug flag
   * @param debug
   */

  void setDebug(bool debug) { debug_ = debug; }
  void setMaxStepSize(double step) { max_step_size_ = step; }
  void setPathLimit(double limit) { path_limit_ = limit; }

  /** Method to extrapolate to a target surface given a set of
   BoundTrackParameters
   *
   @param pars: Bound Track Parameters
   @param target_target_surface: The target surface
   @return optional with BoundTrackParameters
  */

  using PropagatorOptions =
      typename propagator_t::template Options<ActionList>;

  std::optional<Acts::BoundTrackParameters> extrapolate(
      const Acts::BoundTrackParameters pars,
      const std::shared_ptr<Acts::Surface>& target_surface) {
    auto intersection = target_surface->intersect(gctx_, pars.position(gctx_),
                                                  pars.direction());

    PropagatorOptions p_options(gctx_, mctx_);
    if (max_step_size_ > 0) p_options.stepping.maxStepSize = max_step_size_;
    if (path_limit_ > 0) p_options.pathLimit = path_limit_;

    p_options.direction = intersection[0].pathLength() >= 0
                              ? Acts::Direction::Forward()
                              : Acts::Direction::Backward();

    auto result = propagator_.propagate(pars, *target_surface, p_options);

    // CHECK THE EXTRAPOLATION COVARIANCE MATRIX

    if (debug_) {
      if (result.ok()) {
        std::cout << "INITIAL COV MATRIX\n";
        std::cout << (*(pars.covariance())) << std::endl;

        std::cout << "FINAL COV MATRIX\n";
        auto opt_pars = *result->endParameters;
        std::cout << *(opt_pars.covariance()) << std::endl;
      }
    }

    if (result.ok())
      return *result->endParameters;
    else
      return std::nullopt;
  }  // end of extrapolate()

  /** Method to extrapolate to a target surface given a track
   * The method computes which track state is closest to the surface to choose
   which one to use to
   * extrapolate. This method doesn't use a measurement, but whatever first/last
   track state is defined.
   @param track: An Acts::Track
   @param target_surface: The target surface
   @return optional containing the bound track parameters.
   **/

  template <class track_t>
  std::optional<Acts::BoundTrackParameters> extrapolate(
      track_t track, const std::shared_ptr<Acts::Surface>& target_surface) {
    if (debug_) {
      std::cout << "[TrackExtrapolatorTool] extrapolate START\n";
      std::cout << "[TrackExtrapolatorTool]   track.nTrackStates() = "
                << track.nTrackStates() << std::endl;
      std::cout << "[TrackExtrapolatorTool]   target_surface = "
                << target_surface.get() << std::endl;
    }

    if (track.nTrackStates() == 0) {
      return std::nullopt;
    }

    // Use ACTS's built-in helper to find the measurement track state
    // (first or last) that is closest to the target surface. This correctly
    // handles holes and material-only states which lack filtered parameters.
    auto stateResult = Acts::findTrackStateForExtrapolation(
        gctx_, track, *target_surface,
        Acts::TrackExtrapolationStrategy::firstOrLast);

    if (!stateResult.ok()) {
      return std::nullopt;
    }

    const auto& ts = stateResult->first;
    const auto& surface = ts.referenceSurface();

    Acts::BoundVector params;
    Acts::BoundMatrix cov;

    if (ts.hasSmoothed()) {
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Using smoothed parameters\n";
      params = ts.smoothed();
      cov = ts.smoothedCovariance();
    } else if (ts.hasFiltered()) {
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Using filtered parameters\n";
      params = ts.filtered();
      cov = ts.filteredCovariance();
    } else {
      return std::nullopt;
    }

    if (debug_) {
      std::cout << "Surface::"
                << surface.localToGlobalTransform(gctx_).translation()
                << std::endl;
      std::cout << "HasSmoothed::" << ts.hasSmoothed() << std::endl;
      std::cout << "Parameters::" << params.transpose() << std::endl;
    }

    auto part_hypo{Acts::ParticleHypothesis::electron()};
    Acts::BoundTrackParameters sp(surface.getSharedPtr(), params, cov,
                                  part_hypo);
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   calling extrapolate(BTP)...\n";
    auto result = extrapolate(sp, target_surface);
    if (debug_) std::cout << "[TrackExtrapolatorTool]   extrapolate DONE\n";
    return result;
  }

  /**
   ** Create an ldmx::TrackState to the extrapolated position
   @param track: Acts::Track
   @param target_surface: extrapolation surface
   @param ts: ldmx::Track::TrackState
   @param type: TrackStateType
   @return optional boolean to check if there was a problem in the extrapolation
   *
   */
  template <class track_t>
  std::optional<Acts::BoundTrackParameters> extrapolateToEcal(
      track_t track, const std::shared_ptr<Acts::Surface>& target_surface) {
    // get last track state on the track.
    // Now.. I'm taking whatever it is. I'm not checking here if it is a
    // measurement.

    auto& tsc = track.container().trackStateContainer();
    auto begin = track.trackStates().begin();
    auto ts_last = *begin;
    const auto& surface = (ts_last).referenceSurface();
    const auto& smoothed = (ts_last).smoothed();
    const auto& cov = (ts_last).smoothedCovariance();

    // Get the BoundTrackStateParameters
    // assume electron for now
    auto part_hypo{Acts::ParticleHypothesis::electron()};

    Acts::BoundTrackParameters state_parameters(surface.getSharedPtr(),
                                                smoothed, cov, part_hypo);

    // One can also use directly the extrapolate method
    PropagatorOptions p_options(gctx_, mctx_);
    auto result =
        propagator_.propagate(state_parameters, *target_surface, p_options);

    if (result.ok())
      return *result->endParameters;
    else
      return std::nullopt;
  }

  /**
   ** Create an ldmx::TrackState to the extrapolated position
   @param track: Acts::Track
   @param target_surface: extrapolation surface
   @param ts: ldmx::Track::TrackState
   @param type: TrackStateType
   @return boolean to check if there was a problem in the extrapolation
   *
   */

  template <class track_t>
  bool trackStateAtSurface(track_t track,
                           const std::shared_ptr<Acts::Surface>& target_surface,
                           ldmx::Track::TrackState& ts,
                           ldmx::TrackStateType type) {
    if (debug_) {
      std::cout << "[TrackExtrapolatorTool] trackStateAtSurface START\n";
      std::cout << "[TrackExtrapolatorTool]   target_surface = "
                << target_surface.get() << std::endl;
      std::cout << "[TrackExtrapolatorTool]   track.nTrackStates() = "
                << track.nTrackStates() << std::endl;
      std::cout << "[TrackExtrapolatorTool]   TrackStateType = "
                << static_cast<int>(type) << std::endl;
      std::cout << "[TrackExtrapolatorTool]   About to call extrapolate...\n";
    }

    auto opt_pars = extrapolate(track, target_surface);

    if (debug_) {
      std::cout << "[TrackExtrapolatorTool]   extrapolate returned, "
                   "opt_pars.has_value() = "
                << opt_pars.has_value() << std::endl;
    }

    if (opt_pars) {
      if (debug_) {
        Acts::Vector3 surf_loc = target_surface->localToGlobalTransform(gctx_).translation();
        std::cout << "[TrackExtrapolatorTool]   Surface location: ("
                  << surf_loc(0) << ", " << surf_loc(1) << ", " << surf_loc(2)
                  << ")\n";
      }

      ts = tracking::sim::utils::makeTrackState(gctx_, *opt_pars, type);
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   trackStateAtSurface SUCCESS\n";
      return true;
    } else {
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   trackStateAtSurface FAILED - "
                     "opt_pars is empty\n";
      return false;
    }
  }

 private:
  propagator_t propagator_;
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext mctx_;
  bool debug_{false};
  double max_step_size_{-1};
  double path_limit_{-1};
};

}  // namespace reco
}  // namespace tracking
