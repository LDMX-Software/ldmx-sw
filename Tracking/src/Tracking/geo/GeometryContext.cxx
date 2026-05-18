#include "Tracking/geo/GeometryContext.h"

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

namespace tracking::geo {

const std::string GeometryContext::NAME = "TrackingGeometryContext";

GeometryContext::GeometryContext() : framework::ConditionsObject(NAME) {
  acts_gc_ = this;
}

const Acts::GeometryContext& GeometryContext::get() const { return acts_gc_; }

void GeometryContext::loadTransformations(const tgSurfMap& surf_map) {
  // Always clear the map before reloading the transformations.
  alignment_map_.clear();

  for (auto entry : surf_map) {
    alignment_map_[entry.first] =
        static_cast<const DetectorElement*>(
            (entry.second)->associatedDetectorElement())
            ->uncorrectedTransform();
  }
}
/*
 Some testing functionality
 deltaT = (tu, tv, tw)
 deltaR = (ru, rv, rw)

         /  1    -rw   rv  \
deltaR = |  rw    1   -ru  |
         \ -rv    ru   1   /

 "active"  means "rotate, then translate"
 "passive" means "translate, then rotate"
*/

void GeometryContext::addAlignCorrection(unsigned int sensorId,
                                         const Acts::Vector3 deltaT,
                                         const Acts::Vector3 deltaR,
                                         bool active) {
  Acts::Translation3 delta_translation{deltaT};
  Acts::RotationMatrix3 rot = deltaRot(deltaR);
  Acts::Transform3 correction(delta_translation * rot);

  // Add the correction to the alignment map

  if (alignment_map_.count(sensorId) < 1) {
    EXCEPTION_RAISE("BadGeometry",
                    "GeometryContext:: could not addAlignCorrection");
  }

  if (active) {
    // qaligned = dR*R(t0 + dt0).
    // ==> rotate, then translate.
    alignment_map_[sensorId].rotate(correction.rotation());
    alignment_map_[sensorId].translate(correction.translation());
  } else {
    alignment_map_[sensorId].translate(correction.translation());
    alignment_map_[sensorId].rotate(correction.rotation());
  }
}

class GeometryContextProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Create the context conditions object
   *
   * @param[in] name the name of this provider
   * @param[in] tagname the name of the tag generation of this condition
   * @param[in] parameters configuration parameters from python
   * @param[in] process reference to the running process object
   */
  GeometryContextProvider(const std::string& name, const std::string& tagname,
                          const framework::config::Parameters& parameters,
                          framework::Process& process)
      : framework::ConditionsObjectProvider(GeometryContext::NAME, tagname,
                                            parameters, process) {}

  /**
   * Get the context as a conditions object
   *
   * We just create a new context without any parameters and return
   * the unlimited interval of validity.
   *
   * @param[in] context EventHeader for the event context
   * @returns new context and unlimited interval of validity
   */
  std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) final override {
    return std::make_pair<const framework::ConditionsObject*,
                          framework::ConditionsIOV>(
        new GeometryContext(), framework::ConditionsIOV(true, true));
  }
};

}  // namespace tracking::geo

DECLARE_CONDITIONS_PROVIDER(tracking::geo::GeometryContextProvider)
