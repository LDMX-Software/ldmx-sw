#include "Tracking/geo/MagneticFieldContext.h"

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

namespace tracking::geo {

const std::string MagneticFieldContext::NAME = "TrackingMagneticFieldContext";

MagneticFieldContext::MagneticFieldContext()
  : framework::ConditionsObject(NAME) {}

const Acts::MagneticFieldContext& MagneticFieldContext::get() const {
  return magnetic_field_context_;
}

class MagneticFieldContextProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Create the context conditions object
   *
   * @param[in] name the name of this provider
   * @param[in] tagname the name of the tag generation of this condition
   * @param[in] parameters configuration parameters from python
   * @param[in] process reference to the running process object
   */
  MagneticFieldContextProvider(const std::string& name, const std::string& tagname,
                               const framework::config::Parameters& parameters,
                               framework::Process& process) 
    : framework::ConditionsObjectProvider(MagneticFieldContext::NAME, tagname, parameters, process) {}

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
        new MagneticFieldContext(),
        framework::ConditionsIOV(true, true)
    );
  }
 
};

}

DECLARE_CONDITIONS_PROVIDER_NS(tracking::geo, MagneticFieldContextProvider)
