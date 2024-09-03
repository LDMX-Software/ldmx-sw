// Tagger tracker: vol=2 , layer = [2,4,6,8,10,12,14], sensor=[1,2]
// Recoil tracker: vol=3 , layer = [2,4,6,8,10,12],
// sensor=[1,2,3,4,5,6,7,8,9,10]

#pragma once

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"

//--- C++ ---//
#include <memory>
#include <random>

//--- LDMX ---//
#include "Tracking/Reco/TrackingGeometryUser.h"

//--- ACTS ---//

// Utils and Definitions
#include "Acts/Definitions/Common.hpp"
//#include "Acts/Definitions/TrackParametrization.hpp"
//#include "Acts/Definitions/Units.hpp"
//#include "Acts/EventData/TrackParameters.hpp"
//#include "Acts/EventData/detail/TransformationFreeToBound.hpp"
#include "Acts/Utilities/Logger.hpp"

// geometry
#include "Acts/Geometry/GeometryContext.hpp"

// magfield
//#include "Acts/MagneticField/MagneticFieldContext.hpp"
//#include "Acts/MagneticField/MagneticFieldProvider.hpp"

// geometry
#include <Acts/Geometry/TrackingGeometry.hpp>

// propagation testing
//#include "Acts/MagneticField/ConstantBField.hpp"
//#include "Acts/Propagator/AbortList.hpp"
//#include "Acts/Propagator/ActionList.hpp"
//#include "Acts/Propagator/DenseEnvironmentExtension.hpp"
//#include "Acts/Propagator/EigenStepper.hpp"
//#include "Acts/Propagator/MaterialInteractor.hpp"
//#include "Acts/Propagator/Navigator.hpp"
//#include "Acts/Propagator/Propagator.hpp"
//#include "Acts/Propagator/StandardAborters.hpp"
//#include "Acts/Propagator/detail/SteppingLogger.hpp"
//#include "Acts/Surfaces/PerigeeSurface.hpp"
//#include "Acts/Utilities/Logger.hpp"

// Kalman Filter

//#include "Acts/EventData/Measurement.hpp"
//#include "Acts/EventData/MultiTrajectory.hpp"
//#include "Acts/EventData/MultiTrajectoryHelpers.hpp"
//#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
//#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
//#include "Acts/TrackFinding/MeasurementSelector.hpp"
//#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
//#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
//#include "Acts/Utilities/CalibrationContext.hpp"

//--- Refit with backward propagation ---//
//#include "Acts/TrackFitting/KalmanFitter.hpp"

//-- Ambiguity Solving --//
//#include "Tracking/Reco/AmbiguitySolver.h"
//#include "Tracking/Reco/ScoreBasedAmbiguitySolver.h"

// GSF
//#include "Acts/TrackFitting/GaussianSumFitter.hpp"
//#include "Acts/Propagator/MultiEigenStepperLoop.hpp"

//--- Tracking ---//
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TrackExtrapolatorTool.h"
#include "Tracking/Sim/IndexSourceLink.h"
#include "Tracking/Sim/MeasurementCalibrator.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- Interpolated magnetic field ---//
#include "Tracking/Sim/BFieldXYZUtils.h"


namespace tracking {
namespace reco {

/**
 * Minimal example of a processor.
 *
 * This processor will loop over all of the ECal hits in an event and
 * print out their details.
 */
class ScoreBasedAmbiguitySolver final : public TrackingGeometryUser {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  ScoreBasedAmbiguitySolver(const std::string &name, framework::Process &process);

  /// Destructor
  ~ScoreBasedAmbiguitySolver();

  void onProcessStart() override;

  /**
   * onNewRun is the first function called for each processor
   * *after* the conditions are fully configured and accessible.
   * This is where you could create single-processors, multi-event
   * calculation objects.
   */
  void onNewRun(const ldmx::RunHeader &rh) override;

  /**
   *
   */
  void onProcessEnd() override;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * The user specified parameters that are availabed are defined
   * in the python configuration class. Look at the my_processor.py
   * module of the EventProc python for the python structure.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) override;

  /**
   * Process the event and put new data products into it.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event) override;

  private:
    struct DetectorConfig {
    double hitsScoreWeight = 0;
    double holesScoreWeight = 0;
    double outliersScoreWeight = 0;
    double otherScoreWeight = 0;

    int minHits = 0;
    int maxHits = 0;
    int maxHoles = 0;
    int maxOutliers = 0;
    int maxSharedHits = 0;

    /// if true, the shared hits are considered as bad hits for this detector
    bool sharedHitsFlag = false;

    int detectorId = 0;

    /// a list of values from  0 to 1, the higher number of hits, higher value
    /// in the list is multiplied to ambuiguity score applied only if
    /// useAmbiguityFunction is true
    std::vector<double> factorHits = {1.0};

    /// a list of values from  0 to 1, the higher number of holes, lower value
    /// in the list is multiplied to ambuiguity score applied only if
    /// useAmbiguityFunction is true
    std::vector<double> factorHoles = {1.0};
  };

  struct TrackFeatures {
    int nHits = 0;
    int nHoles = 0;
    int nOutliers = 0;
    int nSharedHits = 0;
  };

  struct MeasurementInfo {
    std::size_t iMeasurement = 0;
    std::size_t detectorId = 0;
    bool isOutlier = false;
  };

    std::map<int, int> volumeMap_;
    std::vector<DetectorConfig> detectorConfigs_;
    
    /// minimum score for any track
    double minScore_{0};
    /// minimum score for shared tracks
    double minScoreSharedTracks_{0};
    /// maximum number of shared tracks per measurement
    int maxSharedTracksPerMeasurement_{10};
    /// maximum number of shared hit per track
    int maxShared_{5};

    double pTMin_{0 * Acts::UnitConstants::GeV};
    double pTMax_{1e5 * Acts::UnitConstants::GeV};

    double phiMin_{-M_PI * Acts::UnitConstants::rad};
    double phiMax_{M_PI * Acts::UnitConstants::rad};

    double etaMin_{-5};
    double etaMax_{5};

    // if true, the ambiguity score is computed based on a different function.
    bool useAmbiguityFunction_{false};

    bool verbose_{false};


    std::string out_trk_collection_{"TaggerTracksClean"};

    std::string trackCollection_{"TaggerTracks"};

    std::string measCollection_{"DigiTaggerSimHits"};

    enableLogging("ScoreBasedAmbiguitySolver")

  /// @param tracks is the input track container
  /// @param sourceLinkHash is the  source links
  /// @param sourceLinkEquality is the equality function for the source links
  /// @param trackFeaturesVectors is the trackFeatures map from detector ID to trackFeatures
  /// @return a vector of the initial state of the tracks
  template <typename source_link_hash_t, typename source_link_equality_t, typename geometry_t>
std::vector<std::vector<ScoreBasedAmbiguitySolver::MeasurementInfo>> computeInitialState(
      std::vector<ldmx::Track> tracks,  std::vector<ldmx::Measurement> meas_coll,
      source_link_hash_t sourceLinkHash,
      source_link_equality_t sourceLinkEquality, geometry_t& tg, 
      std::vector<std::vector<ScoreBasedAmbiguitySolver::TrackFeatures>>& trackFeaturesVectors) const;

  /// Compute the score of each track.
  ///
  /// @param tracks is the input track container
  /// @param trackFeaturesVectors is the trackFeatures map from detector ID to trackFeatures
  std::vector<double> simpleScore(
      const std::vector<ldmx::Track> tracks,
      const std::vector<std::vector<TrackFeatures>>& trackFeaturesVectors) const;


  /// @brief Remove tracks that are not good enough
  /// @param tracks is the input track container
  /// @param measurementsPerTrack is the list of measurements for each track
  /// @param trackFeaturesVectors is the map of detector id to trackFeatures for each track
  /// @return a vector of IDs of the tracks we want to keep
  std::vector<int> solveAmbiguity(
     std::vector<ldmx::Track> tracks,
    const std::vector<std::vector<ScoreBasedAmbiguitySolver::MeasurementInfo>>& measurementsPerTrack,
    const std::vector<std::vector<ScoreBasedAmbiguitySolver::TrackFeatures>>& trackFeaturesVectors) const;


  /// @brief Remove tracks that are not good enough based on cuts
  /// @param trackScore is the score of each track
  /// @param trackFeaturesVectors is the trackFeatures map for each track
  /// @param measurementsPerTrack is the list of measurements for each track
  /// @return a vector of IDs of the tracks we want to keep
  std::vector<bool> getCleanedOutTracks(
    const std::vector<double>& trackScore,
    const std::vector<std::vector<TrackFeatures>>& trackFeaturesVectors,
    const std::vector<std::vector<MeasurementInfo>>& measurementsPerTrack)
    const;

  


};  // MyProcessor

}  // namespace recon
}