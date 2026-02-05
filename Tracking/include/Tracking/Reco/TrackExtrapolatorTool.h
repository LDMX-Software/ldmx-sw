#pragma once

#include <iostream>
#include <iterator>
#include <optional>

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Tracking/Event/Track.h"
#include "Tracking/Sim/TrackingUtils.h"

using ActionList =
    Acts::ActionList<Acts::detail::SteppingLogger, Acts::MaterialInteractor>;
using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;

namespace tracking {
namespace reco {

template <class propagator_t>
class TrackExtrapolatorTool {
 public:
  // The geometry context should be already in the propagator options...
  TrackExtrapolatorTool(propagator_t propagator,
                        const Acts::GeometryContext& gctx,
                        const Acts::MagneticFieldContext& mctx)
      : propagator_(std::move(propagator)) {
    gctx_ = gctx;
    mctx_ = mctx;
  }

  /**
   * Turn on/off internal debug flag
   * @param debug
   */

  void setDebug(bool debug) { debug_ = debug; }

  /** Method to extrapolate to a target surface given a set of
   BoundTrackParameters
   *
   @param pars: Bound Track Parameters
   @param target_target_surface: The target surface
   @return optional with BoundTrackParameters
  */

  using PropagatorOptions =
      typename propagator_t::template Options<ActionList, AbortList>;

  std::optional<Acts::BoundTrackParameters> extrapolate(
      const Acts::BoundTrackParameters pars,
      const std::shared_ptr<Acts::Surface>& target_surface) {
    auto intersection = target_surface->intersect(gctx_, pars.position(gctx_),
                                                  pars.direction());

    PropagatorOptions p_options(gctx_, mctx_);

    p_options.direction = intersection.intersections()[0].pathLength() >= 0
                              ? Acts::Direction::Forward
                              : Acts::Direction::Backward;

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

    // get first and last track state on surface
    if (debug_)
      std::cout
          << "[TrackExtrapolatorTool]   Getting outermost track state...\n";
    auto outermost = *(track.trackStatesReversed().begin());
    if (debug_)
      std::cout
          << "[TrackExtrapolatorTool]   Getting innermost track state...\n";
    auto begin = track.trackStatesReversed().begin();
    std::advance(begin, track.nTrackStates() - 1);
    auto innermost = *begin;
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   Got innermost and outermost "
                   "track states\n";

    // I'm checking which track state is closer to the origin of the target
    // surface to decide from where to start the extrapolation to the surface. I
    // use the coordinate along the beam axis.
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   Calculating distances...\n";
    double first_dis = std::abs(
        innermost.referenceSurface().transform(gctx_).translation()(0) -
        target_surface->transform(gctx_).translation()(0));

    double last_dis = std::abs(
        outermost.referenceSurface().transform(gctx_).translation()(0) -
        target_surface->transform(gctx_).translation()(0));
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   first_dis = " << first_dis
                << ", last_dis = " << last_dis << std::endl;

    // This is the track state to use for the extrapolation

    const auto& ts = first_dis < last_dis ? innermost : outermost;
    if (debug_) std::cout << "[TrackExtrapolatorTool]   Selected track state\n";

    // Get the BoundTrackStateParameters

    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   Getting reference surface...\n";
    const auto& surface = ts.referenceSurface();
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   Checking hasSmoothed...\n";
    bool has_smoothed = ts.hasSmoothed();
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   has_smoothed = " << has_smoothed
                << std::endl;

    // Use smoothed parameters if available, otherwise use filtered
    Acts::BoundVector params;
    Acts::BoundMatrix cov;

    if (has_smoothed) {
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Using smoothed parameters...\n";
      params = ts.smoothed();
      cov = ts.smoothedCovariance();
    } else {
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Using filtered parameters...\n";
      params = ts.filtered();
      cov = ts.filteredCovariance();
    }
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   Got all track state components\n";

    if (debug_) {
      std::cout << "Surface::" << surface.transform(gctx_).translation()
                << std::endl;
      std::cout << "HasSmoothed::" << has_smoothed << std::endl;
      std::cout << "Parameters::" << params.transpose() << std::endl;
    }
    // mg Aug 2024 ... v36 takes the particle...assume electron
    if (debug_)
      std::cout
          << "[TrackExtrapolatorTool]   Creating BoundTrackParameters...\n";
    auto part_hypo{Acts::SinglyChargedParticleHypothesis::electron()};
    Acts::BoundTrackParameters sp(surface.getSharedPtr(), params, cov,
                                  part_hypo);
    if (debug_)
      std::cout << "[TrackExtrapolatorTool]   BoundTrackParameters created, "
                   "calling extrapolate(BTP)...\n";
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
    auto part_hypo{Acts::SinglyChargedParticleHypothesis::electron()};

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
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Getting surface location...\n";
      // Reference point
      Acts::Vector3 surf_loc = target_surface->transform(gctx_).translation();
      ts.ref_x_ = surf_loc(0);
      ts.ref_y_ = surf_loc(1);
      ts.ref_z_ = surf_loc(2);
      if (debug_) {
        std::cout << "[TrackExtrapolatorTool]   Surface location: ("
                  << surf_loc(0) << ", " << surf_loc(1) << ", " << surf_loc(2)
                  << ")\n";
      }

      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Getting parameters...\n";
      // Parameters
      ts.params_ =
          tracking::sim::utils::convertActsToLdmxPars((*opt_pars).parameters());
      if (debug_) std::cout << "[TrackExtrapolatorTool]   Parameters set\n";

      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Getting covariance...\n";
      // Covariance
      const Acts::BoundMatrix& trk_cov = *((*opt_pars).covariance());
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Covariance matrix obtained\n";
      tracking::sim::utils::flatCov(trk_cov, ts.cov_);
      if (debug_)
        std::cout << "[TrackExtrapolatorTool]   Covariance flattened\n";

      ts.ts_type_ = type;
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
};

}  // namespace reco
}  // namespace tracking
