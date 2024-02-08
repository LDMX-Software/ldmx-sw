#include "Tracking/geo/GeometryContext.h"
#include "Tracking/geo/DetectorElement.h"

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

#include <stdexcept> // Include for std::logic_error

namespace tracking::geo {

const std::string GeometryContext::NAME = "TrackingGeometryContext";

GeometryContext::GeometryContext()
    : framework::ConditionsObject(NAME) {
  acts_gc_ = this;
}

const Acts::GeometryContext& GeometryContext::get() const {
  return acts_gc_;
}

void GeometryContext::loadTransformations(const tgSurfMap& surf_map) { 

  //Always clear the map before reloading the transformations. 
  alignment_map.clear();

  for (auto entry : surf_map) {
    alignment_map[entry.first] = static_cast<const DetectorElement*>((entry.second)->associatedDetectorElement())->uncorrectedTransform();
  }

}

// Some testing functionality
  
  //deltaT = (tu, tv, tw)
  //deltaR = (ru, rv, rw)
  
  //         /  1    -rw   rv  \
  //deltaR = |  rw    1   -ru  |
  //         \ -rv    ru   1   /
  
void GeometryContext::addAlignCorrection(unsigned int sensorId,
                        const Acts::Vector3 deltaT,
                        const Acts::Vector3 deltaR) {
  

  Acts::Translation3 deltaTranslation{deltaT};
  Acts::RotationMatrix3 rot = deltaRot(deltaR);
  Acts::Transform3 correction(deltaTranslation * rot);
  
  // Add the correction to the alignment map

  if (alignment_map.count(sensorId) < 1)  {
    throw std::logic_error("GeometryContext:: could not addAlignCorrection");
  }


  // qaligned = dR*R(t0 + dt0). 
  alignment_map[sensorId].rotate(correction.rotation());
  alignment_map[sensorId].translate(correction.translation());

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
    : framework::ConditionsObjectProvider(GeometryContext::NAME, tagname, parameters, process) {}

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
    return std::make_pair<const framework::ConditionsObject*, framework::ConditionsIOV>(
        new GeometryContext(),
        framework::ConditionsIOV(true, true)
    );
  }
 
};

}

DECLARE_CONDITIONS_PROVIDER_NS(tracking::geo, GeometryContextProvider)
