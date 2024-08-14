#ifndef LDMXMEASUREMENTCALIBRATOR_H_
#define LDMXMEASUREMENTCALIBRATOR_H_

#include <vector>

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/LdmxSpacePoint.h"



/** The measurement calibrator can be a function or a class/struct able to
 retrieve the sim hits container.
 *  It gets by CKF passing the propagated TrackState. The measurement calibrator
 gets called for every propagation and a new track state is passed which
 contains the source link.
 *  The measurement calibrator unpacks the source link from the TrackState and
 casts to the implemented source link (in ldmx case we use an IndexSourceLink).
 *  Retrieve the index from the source link and look up the hit in the sim
 container. Then fill up the trackState with the projected measurement

 //ts.calibrated().block<2>(0) = surface.globalToLocal(simHit.position)
 //ts.clabiratedCov = simHit cov
 //ts.data().measDim = 2 Matrix H = makeProj() ts.setProjector(H)   Where ts is
 the proxyTrackState  H Matrix projector (this is only for u and v)
 //(1 0 0 0 0 0)  
 //(0 1 0 0 0 0)
 */

/// Calibrator to convert an index source link to a measurement.
/// The measurement could be a space point or a local cluster on surface.
/// TODO: use local measurements instead of global space points.

namespace tracking {
namespace sim {

class LdmxMeasurementCalibrator {
 public:
  /// Construct an invalid calibrator. Required to allow copying.
  LdmxMeasurementCalibrator() = default;

  // The calibrator needs to access the sim hit container
  LdmxMeasurementCalibrator(
      const std::vector<ldmx::Measurement>& measurements) {
    m_measurements = &measurements;
  }

  /// Find the measurement corresponding to the source link. Uses a 2D
  /// measurement, cov-matrix and projection
  ///
  /// @tparam parameters_t Track parameters type
  /// @param gctx The geometry context (unused)
  /// @param trackState The track state to calibrate
  template <typename traj_t>
  void calibrate(
		 const Acts::GeometryContext& /*gctx*/,
		 const Acts::CalibrationContext& /*cctx*/,
		 const ActsExamples::IndexSourceLink& sourceLink/*sourceLink*/,
		 traj_t::TrackStateProxy
		 trackState) const {

    assert(m_measurements and
           "Undefined measurement container in LdmxMeasurementCalibrator");
    assert((sourceLink.index() < m_measurements->size()) and
           "Source link index is outside the container bounds in "
           "LdmxMeasurementCalibrator");

    auto meas = m_measurements->at(sourceLink.index());
    Acts::Vector2 local_pos{meas.getLocalPosition()[0],
                            meas.getLocalPosition()[1]};
    auto tsIndexType=trackState.index();
    auto tsCal{trackState.template calibrated<2>()};
    auto tsCalCov{trackState.template calibratedCovariance<2>()};
    //    auto trackStateIndex=trackState.index();
    //    trackState.calibrated<1>(tsIndexType).setZero();
    tsCal.setZero();
    //    tsCal.head<2>() = local_pos;
    trackState.template calibrated<2>().template head<2>() = local_pos;
    Acts::SquareMatrix2 local_cov;
    local_cov.setZero();
    local_cov(0, 0) = meas.getLocalCovariance()[0];
    local_cov(1, 1) = meas.getLocalCovariance()[1];
    tsCalCov.setZero();
    tsCalCov.block<2,2>(0,0)=local_cov;

    Acts::ActsMatrix<2, 6> projector;
    projector.setZero();
    projector(0, 0) = 1.;
    projector(1, 1) = 1.;

    trackState.setProjector(projector);
  }

  /// Find the measurement corresponding to the source link.
  /// Uses a 1D measurement, cov-matrix and projection
  ///
  /// @tparam parameters_t Track parameters type
  /// @param gctx The geometry context (unused)
  /// @param trackState The track state to calibrate
  template <typename traj_t>
  void calibrate_1d(
		    const Acts::GeometryContext& /*gctx*/,
		    const Acts::CalibrationContext& /*cctx*/,
		    //		    const ActsExamples::IndexSourceLink& sourceLink/*sourceLink*/,		    
		    const Acts::SourceLink& dummySourceLink/*sourceLink*/,
		    typename traj_t::TrackStateProxy
		    trackState) const {
    // Use by value - life management is not working properly
    //mg Aug 2024 ... in v36, this is an argument
    ActsExamples::IndexSourceLink sourceLink =
      trackState.getUncalibratedSourceLink().template get<ActsExamples::IndexSourceLink>();
    
    assert(m_measurements and
           "Undefined measurement container in LdmxMeasurementCalibrator");
    assert((sourceLink.index() < m_measurements->size()) and
           "Source link index is outside the container bounds in "
           "LdmxMeasurementCalibrator");

    // std::cout<<"calibrate_1d ==> NMeasurements available=" <<
    // m_measurements->size()<<std::endl;
    auto meas = m_measurements->at(sourceLink.index());

    // You need to explicitly allocate measurements here
    //mg Aug 2024  ...things have changed quite a bit in v36
    trackState.allocateCalibrated(1);
    auto tsIndexType=trackState.index();
    auto tsCal{trackState.template calibrated<1>()};
    auto tsCalCov{trackState.template calibratedCovariance<1>()};
    //    auto trackStateIndex=trackState.index();
    //    trackState.calibrated<1>(tsIndexType).setZero();
    tsCal.setZero();
    tsCal(0) = (meas.getLocalPosition())[0];
    tsCalCov.setZero();
    tsCalCov(0, 0) = (meas.getLocalCovariance())[0];

    Acts::ActsMatrix<2, 6> projector;
    projector.setZero();
    projector(0, 0) = 1.;
    projector(1, 1) = 1.;
    trackState.setProjector(projector.row(0));
  }

  // Function to test the measurement calibrator
  // It takes an user defined source link and returns the information of the
  // linked measurement
  void test(const Acts::GeometryContext& /*gctx*/,
            const ActsExamples::IndexSourceLink& sourceLink) const {
    auto meas = m_measurements->at(sourceLink.index());
    // get the measurement
    std::cout << "Measurement layer::\n" << meas.getLayer() << std::endl;

    Acts::Vector3 global_pos{meas.getGlobalPosition()[0],
                             meas.getGlobalPosition()[1],
                             meas.getGlobalPosition()[2]};
    std::cout << "Measurement global_position::\n" << global_pos << std::endl;

    Acts::Vector2 local_pos{meas.getLocalPosition()[0],
                            meas.getLocalPosition()[1]};
    std::cout << "Measurement local_position::\n" << local_pos << std::endl;
  }

 private:
  // use pointer so the calibrator is copyable and default constructible.
  const std::vector<ldmx::Measurement>* m_measurements = nullptr;
};

}  // namespace sim
}  // namespace tracking

#endif
