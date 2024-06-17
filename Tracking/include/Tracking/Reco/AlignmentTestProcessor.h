#pragma once

#include <string>

//--- LDMX ---//
#include "Tracking/Reco/TrackingGeometryUser.h"
#include "Tracking/geo/GeometryContext.h"

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"

namespace tracking::reco {

class AlignmentTestProcessor final : public TrackingGeometryUser {
 public:
  AlignmentTestProcessor(const std::string& name, framework::Process& process);

  ~AlignmentTestProcessor();

  void onProcessStart() override;

  void onNewRun(const ldmx::RunHeader& rh) override;

  void onProcessEnd() override;

  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

 private:
  std::unordered_map<unsigned int, Acts::Transform3> alignmentTransforms;

  tracking::geo::GeometryContext test_gctx_;
  // The Trackers Tracking geometry
  std::shared_ptr<geo::TrackersTrackingGeometry> tg_;
};
}  // namespace tracking::reco
