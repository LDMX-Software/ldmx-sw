#pragma once

#include "Framework/ConditionsObject.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

#include "Tracking/Reco/BaseTrackingGeometry.h"

namespace tracking::geo {
class TrackingGeometry : public framework::ConditionsObject {
 public:
  static constexpr const char* CONDITIONS_OBJECT_NAME{"TrackingGeometry"};

  /// Destructor.
  virtual ~TrackingGeometry() = default;

 private:
  TrackingGeometry(const framework::config::Parameters& parameters);

  /// The path to the GDML description of the detector
  std::string detector_{""};
};
}  // namespace tracking::geo
