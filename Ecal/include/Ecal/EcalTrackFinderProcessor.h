/**
 * @file EcalTrackFinderProcessor.h
 * @brief Processor that uses ACTS to fit tracks through ECAL hits
 * @author Tamas Almos Vami (UCSB) 
 */

#ifndef ECAL_ECALTRACKFINDERPROCESSOR_H_
#define ECAL_ECALTRACKFINDERPROCESSOR_H_

// LDMX
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"
#include "Tracking/EigenStepper.h" // help silence and internal warning
#include "Tracking/Sim/IndexSourceLink.h"

// ACTS
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/Material/HomogeneousVolumeMaterial.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"

// C++
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ecal {

/**
 * @class EcalTrackFinderProcessor
 * @brief Uses ACTS framework to fit tracks through ECAL hits with zero B-field
 *
 * This processor:
 * 1. Converts ECAL RecHits to ldmx::Measurement objects
 * 2. Creates unbounded plane surfaces at each ECAL layer
 * 3. Finds straight-line seed tracks through hits
 * 4. Runs ACTS Combinatorial Kalman Filter with zero magnetic field
 * 5. Outputs fitted ldmx::Track objects
 */
class EcalTrackFinderProcessor : public framework::Producer {
 public:
  /**
   * Constructor
   */
  EcalTrackFinderProcessor(const std::string& name,
                           framework::Process& process);

  /**
   * Destructor
   */
  virtual ~EcalTrackFinderProcessor() = default;

  /**
   * Configure the processor
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * Initialize ACTS tracking objects once the detector geometry is known.
   */
  void onNewRun(const ldmx::RunHeader&) override;

  /**
   * Process event to find ECAL tracks
   */
  void produce(framework::Event& event) override;

  /**
   * Print statistics
   */
  void onProcessEnd() override;

 private:
  /**
   * Create ACTS measurement objects from ECAL hits
   */
  std::vector<ldmx::Measurement> createMeasurements(
      const std::vector<ldmx::EcalHit>& hits, std::vector<double>& energies);

  /**
   * Create unbounded plane surfaces at each ECAL layer
   */
  void createEcalSurfaces();

  /**
   * Find seed tracks via straight-line fitting
   */
  std::vector<ldmx::Track> findSeeds(
      const std::vector<ldmx::Measurement>& measurements);

  /**
   * Fit straight line through 3D points
   * Returns: (position, direction, RMS residual)
   */
  std::tuple<Acts::Vector3, Acts::Vector3, double> fitStraightLine(
      const std::vector<Acts::Vector3>& points);

  /**
   * Create geometry ID to source link map for CKF
   */
  std::unordered_multimap<Acts::GeometryIdentifier,
                          acts_examples::IndexSourceLink>
  makeGeoIdSourceLinkMap(const std::vector<ldmx::Measurement>& measurements);

  // Type aliases for ACTS objects
  using EcalPropagator =
      Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
  using TrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer,
                                              Acts::VectorMultiTrajectory>;

  // ACTS objects
  std::unique_ptr<const EcalPropagator> propagator_;
  std::unique_ptr<
      const Acts::CombinatorialKalmanFilter<EcalPropagator, TrackContainer>>
      ckf_;

  // ECAL layer surfaces (layer ID -> surface) - used during geometry building
  std::map<int, std::shared_ptr<Acts::Surface>> layer_surfaces_;

  // Mapping from ECAL layer ID to the GeometryIdentifier assigned by the
  // TrackingGeometryBuilder. These IDs are what the CKF Navigator sees when
  // it reaches a surface, so source links must be keyed by these IDs.
  std::map<int, Acts::GeometryIdentifier> layer_geo_ids_;

  // Reference surface (ECAL front face)
  std::shared_ptr<Acts::Surface> reference_surface_;

  // ACTS tracking geometry for ECAL
  std::shared_ptr<const Acts::TrackingGeometry> tracking_geometry_;

  // Rotation matrix for surface orientation (u=Y, v=Z, normal=X in ACTS)
  Acts::RotationMatrix3 surf_rotation_;

  // Configuration parameters
  std::string rec_coll_name_{"EcalRecHits"};
  std::string rec_pass_name_{""};
  std::string out_track_collection_{"EcalTracks"};
  int min_hits_{3};              // Minimum hits per track
  double max_chi2_{10.0};        // Outlier chi2 cut
  double cell_resolution_{1.5};  // ECAL cell resolution [mm]
  bool debug_{false};            // ACTS debug logging

  // Seed finding parameters
  double max_seed_rms_{10.0};     // Max RMS for seed fit [mm]
  double min_momentum_{50.0};     // Min momentum estimate [MeV]
  double max_momentum_{10000.0};  // Max momentum estimate [MeV]

  // ROC cone energy collection
  bool use_roc_energy_{true};  // Use 68% containment cone for track energy
  std::string roc_file_name_;  // Path to ROC containment radii CSV file
  std::vector<std::vector<float>> roc_range_values_;  // ROC data from file

  // Geometry
  const ldmx::EcalGeometry* geometry_{nullptr};

  // Statistics
  int nevents_{0};
  int ntracks_{0};
  int nseeds_{0};
  double processing_time_{0.0};
};

}  // namespace ecal

#endif  // ECAL_ECALTRACKFINDERPROCESSOR_H_
