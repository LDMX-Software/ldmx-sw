#include "Tracking/geo/CalibrationContext.h"

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

namespace tracking::geo {

const std::string CalibrationContext::NAME = "TrackingCalibrationContext";

CalibrationContext::CalibrationContext() : framework::ConditionsObject(NAME) {}

const Acts::CalibrationContext& CalibrationContext::get() const {
  return calibration_context_;
}

class CalibrationContextProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Create the context conditions object
   *
   * @param[in] name the name of this provider
   * @param[in] tagname the name of the tag generation of this condition
   * @param[in] parameters configuration parameters from python
   * @param[in] process reference to the running process object
   */
  CalibrationContextProvider(const std::string& name,
                             const std::string& tagname,
                             const framework::config::Parameters& parameters,
                             framework::Process& process)
      : framework::ConditionsObjectProvider(CalibrationContext::NAME, tagname,
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
        new CalibrationContext(), framework::ConditionsIOV(true, true));
  }
};

}  // namespace tracking::geo

DECLARE_CONDITIONS_PROVIDER_NS(tracking::geo, CalibrationContextProvider)
