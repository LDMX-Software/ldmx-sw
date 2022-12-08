#ifndef TRACKING_RECO_DIGITIZATIONPROCESSOR_H_
#define TRACKING_RECO_DIGITIZATIONPROCESSOR_H_


//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//--- ACTS ---//
#include "Acts/Digitization/CartesianSegmentation.hpp"
#include "Acts/Digitization/DigitizationModule.hpp"
#include "Acts/Digitization/PlanarModuleStepper.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Definitions/Units.hpp"

//---  DD4hep ---//
#include "DD4hep/Detector.h"

//--- LDMX ---//
#include "Tracking/Reco/TrackersTrackingGeometry.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- ACTS ---//
#include "Acts/Definitions/Units.hpp"

//--- C++ ---//
#include <random>

#include "Tracking/Reco/TrackersTrackingGeometry.h"


namespace tracking {
namespace reco {

class DigitizationProcessor : public framework::Producer {
 public:
  DigitizationProcessor(const std::string &name, framework::Process &process);
  ~DigitizationProcessor();

  void onProcessStart() final override;
  void onProcessEnd() final override;

  void configure(framework::config::Parameters &parameters) final override ;
      
  void produce(framework::Event &event);

  /// The detector
  dd4hep::Detector* detector_{nullptr};
  Acts::GeometryContext gctx_;

  void digitizeHits(const std::vector<ldmx::SimTrackerHit> &sim_hits, std::vector<ldmx::LdmxSpacePoint*>& ldmxsps);

  //TODO avoid copies and use references
  bool mergeSimHits(const std::vector<ldmx::SimTrackerHit>& sim_hits, std::vector<ldmx::SimTrackerHit>& merged_hits);
  bool mergeHits(const std::vector<ldmx::SimTrackerHit>& sihits, std::vector<ldmx::SimTrackerHit>& mergedHits);
  
 private:

  
  std::shared_ptr<tracking::reco::TrackersTrackingGeometry> ldmx_tg;
  std::string hit_collection_;
  std::string out_collection_;
  float minEdep_; // minimum deposited energy cut
  int trackID_;   // select a particular track ID
  bool mergeHits_; // run sim hit merging before running digitization
  bool debug_{false};
  bool do_smearing_{true};
  /// u-direction sigma
  double sigma_u_{0};

  /// v-direction sigma
  double sigma_v_{0};


  //--- Smearing ---//

  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;
  
}; //Digitization Processor
}//reco
}//tracking

#endif
