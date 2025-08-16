
#include "Framework/ConditionsObject.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Process.h"
#include "Tracking/geo/GeometryContext.h"
#include "Tracking/geo/TrackersTrackingGeometry.h"

namespace tracking::geo {

/**
 * The provider of a tracking geometry
 */
class TrackersTrackingGeometryProvider
    : public framework::ConditionsObjectProvider {
 public:
  TrackersTrackingGeometryProvider(
      const std::string& name, const std::string& tag_name,
      const framework::config::Parameters& parameters,
      framework::Process& process);

  ~TrackersTrackingGeometryProvider() = default;

  /**
   * create the tracking geometry as configured by the input parameters,
   * using the input context to update any run-dependent constants
   *
   * @param[in] context EventHeader with contextual information to pull from
   * @return pair of the created object and the associated interval of validity
   */
  std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) final override;

 private:
  /// the path to the detector we will use for tracking
  std::string detector_;
  double tracker_y_length_;
  double tracker_z_length_;

  /// whether to have debug information or not
};

TrackersTrackingGeometryProvider::TrackersTrackingGeometryProvider(
    const std::string& name, const std::string& tag_name,
    const framework::config::Parameters& parameters,
    framework::Process& process)
    : framework::ConditionsObjectProvider(name, tag_name, parameters, process) {
  detector_ = parameters.get<std::string>("detector");
  tracker_y_length_ = parameters.get<double>("tracker_y_length");
  tracker_z_length_ = parameters.get<double>("tracker_z_length");
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
TrackersTrackingGeometryProvider::getCondition(
    const ldmx::EventHeader& context) {
  /**
   * For now, the tracking geometry is assumed to be accurate for the entire
   * IOV of the GeometryContext.
   */

  /**
   * we need the context for this geometry so we request the parent condition
   */
  auto [condition, iov] =
      requestParentCondition(GeometryContext::NAME, context);

  auto the_context = dynamic_cast<const GeometryContext*>(condition);

  /**
   * return a new trackers tracking geometry, the conditions system handles
   * cleaning up with the `realeaseConditionsObject` function which - by default
   * - simply deletes the allocated conditions object. This probably will
   * confuse linters and debuggers but is the main way to do it in the
   * currently-designed conditions system.
   */
  return std::make_pair(
      new TrackersTrackingGeometry(the_context->get(), detector_,
                                   tracker_y_length_, tracker_z_length_),
      iov);
}
}  // namespace tracking::geo

DECLARE_CONDITIONS_PROVIDER(tracking::geo::TrackersTrackingGeometryProvider);
