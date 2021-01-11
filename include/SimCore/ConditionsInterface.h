/**
 * @file ConditionsInterface
 * @brief Glue class which provides connection between EventProcessor and
 * SensitiveDetectors for conditions information
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef SIMCORE_CONDITIONSINTERFACE_H_
#define SIMCORE_CONDITIONSINTERFACE_H_

// LDMX
#include "Framework/EventProcessor.h"

namespace simcore {

/**
 * @class ConditionsInterface
 * @brief Handle to the conditions system, provided at construction to classes
 * which require it
 */
class ConditionsInterface {
 public:
  ConditionsInterface(framework::EventProcessor* p) : processor_{p} {}

  /**
   * Primary request action for a conditions object If the
   * object is in the cache and still valid (IOV), the
   * cached object will be returned.  If it is not in the cache,
   * or is out of date, the () method will be called to provide the
   * object.
   */
  template <class T>
  const T& getCondition(const std::string& condition_name) {
    if (processor_ == 0) {
      EXCEPTION_RAISE("ConditionUnavailableException",
                      "No conditions system object available in SimCore");
    }
    return processor_->getCondition<T>(condition_name);
  }

 private:
  /**
   * Pointer to the owner processor object
   */
  framework::EventProcessor* processor_;
};

}  // namespace simcore

#endif  // SIMCORE_CONDITIONSINTERFACE_H_
