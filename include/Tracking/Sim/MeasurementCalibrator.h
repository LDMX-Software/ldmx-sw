#ifndef LDMXMEASUREMENTCALIBRATOR_H_
#define LDMXMEASUREMENTCALIBRATOR_H_

#include <vector>
#include "Tracking/Sim/LdmxSpacePoint.h"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Event/Measurement.h"


/** The measurement calibrator can be a function or a class/struct able to retrieve the sim hits container.
 *  It gets by CKF passing the propagated TrackState. The measurement calibrator gets called for every propagation and a new track state is passed which contains the source link.
 *  The measurement calibrator unpacks the source link from the TrackState and casts to the implemented source link (in ldmx case we use an IndexSourceLink).
 *  Retrieve the index from the source link and look up the hit in the sim container. Then fill up the trackState with the projected measurement

 //ts.calibrated().block<2>(0) = surface.globalToLocal(simHit.position)
 //ts.clabiratedCov = simHit cov
 //ts.data().measDim = 2 Matrix H = makeProj() ts.setProjector(H)   Where ts is the proxyTrackState  H Matrix projector (this is only for u and v) 
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
      
      //The calibrator needs to access the sim hit container
    LdmxMeasurementCalibrator(const std::vector<ldmx::Measurement>& measurements) {
      m_measurements = &measurements;
    }
      
      /// Find the measurement corresponding to the source link. Uses a 2D measurement, cov-matrix and projection
      ///
      /// @tparam parameters_t Track parameters type
      /// @param gctx The geometry context (unused)
      /// @param trackState The track state to calibrate
      void calibrate(const Acts::GeometryContext& /*gctx*/,
                     Acts::MultiTrajectory::TrackStateProxy trackState) const {
        const auto& sourceLink =
            static_cast<const ActsExamples::IndexSourceLink&>(trackState.uncalibrated());
        
        assert(m_measurements and
               "Undefined measurement container in LdmxMeasurementCalibrator");
        assert((sourceLink.index() < m_measurements->size()) and
               "Source link index is outside the container bounds in LdmxMeasurementCalibrator");

        auto meas = m_measurements->at(sourceLink.index());
        
        trackState.calibrated().setZero();

        Acts::Vector2 local_pos{meas.getLocalPosition()[0], meas.getLocalPosition()[1]};
        trackState.calibrated().head<2>() = local_pos;
        trackState.data().measdim = 2;
        trackState.calibratedCovariance().setZero();

        Acts::SymMatrix2 local_cov;
        local_cov.setZero();
        local_cov(0,0) = meas.getLocalCovariance()[0];
        local_cov(1,1) = meas.getLocalCovariance()[1];
        trackState.calibratedCovariance().block<2,2>(0,0) = local_cov;

        Acts::ActsMatrix<2,6> projector;
        projector.setZero();
        projector(0,0) = 1.;
        projector(1,1) = 1.;
        
        trackState.setProjector(projector);
        
      }

      /// Find the measurement corresponding to the source link.
      /// Uses a 1D measurement, cov-matrix and projection
      ///
      /// @tparam parameters_t Track parameters type
      /// @param gctx The geometry context (unused)
      /// @param trackState The track state to calibrate
      void calibrate_1d(const Acts::GeometryContext& /*gctx*/,
                        Acts::MultiTrajectory::TrackStateProxy trackState) const {
        const auto& sourceLink =
            static_cast<const ActsExamples::IndexSourceLink&>(trackState.uncalibrated());
        
        assert(m_measurements and
               "Undefined measurement container in LdmxMeasurementCalibrator");
        assert((sourceLink.index() < m_measurements->size()) and
               "Source link index is outside the container bounds in LdmxMeasurementCalibrator");


        //std::cout<<"calibrate_1d ==> NMeasurements available=" << m_measurements->size()<<std::endl;
        auto meas = m_measurements->at(sourceLink.index());
        
        trackState.calibrated().setZero();
        trackState.calibrated()(0) = (meas.getLocalPosition())[0];
        trackState.data().measdim = 1;
        trackState.calibratedCovariance().setZero();
        trackState.calibratedCovariance()(0,0) = (meas.getLocalCovariance())[0];
        
        Acts::ActsMatrix<2,6> projector;
        projector.setZero();
        projector(0,0) = 1.;
        projector(1,1) = 1.;
        trackState.setProjector(projector.row(0));
        
        
        
      }

      //Function to test the measurement calibrator
      //It takes an user defined source link and returns the information of the linked measurement
      void test(const Acts::GeometryContext& /*gctx*/,
                const ActsExamples::IndexSourceLink& sourceLink) const {

        
        auto meas = m_measurements->at(sourceLink.index());
        //get the measurement
        std::cout<<"Measurement layer::\n"<<meas.getLayer()<<std::endl;

        Acts::Vector3 global_pos{meas.getGlobalPosition()[0],
          meas.getGlobalPosition()[1],
          meas.getGlobalPosition()[2]};
        std::cout<<"Measurement global_position::\n"<<global_pos<<std::endl;
        
        Acts::Vector2 local_pos{meas.getLocalPosition()[0],
          meas.getLocalPosition()[1]};
        std::cout<<"Measurement local_position::\n"<<local_pos<<std::endl;
                
      }
    
    
   private:

      // use pointer so the calibrator is copyable and default constructible.
      const std::vector<ldmx::Measurement>* m_measurements = nullptr;
    };

  
  }
}

#endif


