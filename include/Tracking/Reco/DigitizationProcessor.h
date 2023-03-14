#pragma once

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//--- ACTS ---//
#include "Acts/Definitions/Units.hpp"
#include "Acts/Digitization/CartesianSegmentation.hpp"
#include "Acts/Digitization/DigitizationModule.hpp"
#include "Acts/Digitization/PlanarModuleStepper.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"

//--- LDMX ---//
#include "Tracking/Reco/TrackersTrackingGeometry.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- ACTS ---//
#include "Acts/Definitions/Units.hpp"

//--- C++ ---//
#include <random>

#include "Tracking/Reco/TrackersTrackingGeometry.h"

namespace ldmx {
class Measurement;
}

namespace tracking::reco {

class DigitizationProcessor : public framework::Producer {
 public:
  DigitizationProcessor(const std::string& name, framework::Process& process);
  ~DigitizationProcessor() = default;

  void onProcessStart() final override;

  void configure(framework::config::Parameters& parameters) final override;

  void produce(framework::Event& event);

  /// The detector
  Acts::GeometryContext gctx_;

  /**
   * Does basic digitization of SimTrackerHits. For now, this simply uses the
   * global coordinates (SimTrackerHit position) and hit surface to extract
   * the local coordinates.  If specified, the local coordinates are smeared and
   * the global coordinates are updated.
   *
   * @param sim_hits The collection of SimTrackerHits to digitize.
   */
  std::vector<ldmx::Measurement> digitizeHits(
      const std::vector<ldmx::SimTrackerHit>& sim_hits);

  // TODO avoid copies and use references
  bool mergeSimHits(const std::vector<ldmx::SimTrackerHit>& sim_hits,
                    std::vector<ldmx::SimTrackerHit>& merged_hits);
  bool mergeHits(const std::vector<ldmx::SimTrackerHit>& sihits,
                 std::vector<ldmx::SimTrackerHit>& mergedHits);

 private:
  std::shared_ptr<tracking::reco::TrackersTrackingGeometry> ldmx_tg;

  /// The path to the GDML description of the detector
  std::string detector_{""};
  /// Input hit collection to smear.
  std::string hit_collection_;
  /// Output hit collection name.
  std::string out_collection_;
  /// Minimum energy deposition cut.
  double min_e_dep_;
  /// Select a particular track ID
  int track_id_;
  /// Merge the sim hits before digitizing.
  bool merge_hits_{false};
  /// Flag to enable/disable smearing.
  bool do_smearing_{true};
  /// u-direction sigma
  double sigma_u_{0};
  /// v-direction sigma
  double sigma_v_{0};

  //--- Smearing ---//

  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;

};  // Digitization Processor
}  // namespace tracking::reco
