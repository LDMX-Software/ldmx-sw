#ifndef HCAL_HCALTRIGGERGEOMETRY_H_
#define HCAL_HCALTRIGGERGEOMETRY_H_

// LDMX
#include "DetDescr/HcalDigiID.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/HcalTriggerID.h"
#include "Framework/ConditionsObject.h"

// STL
#include <map>
#include <vector>

namespace ldmx {
class HcalGeometry;
}

namespace hcal {

/**
 * @class HcalTriggerGeometry
 * @brief defines the relationship between HCal strips and CMB quad trigger
 * primitives and provides geometry information for trigger primitives
 */
class HcalTriggerGeometry : public framework::ConditionsObject {
 public:
  static constexpr const char* CONDITIONS_OBJECT_NAME{"HcalTriggerGeometry"};

  HcalTriggerGeometry(const ldmx::HcalGeometry* hcalGeom = 0);

  /**
   * Returns the set of precision (full-granularity/DAQ) cells which are
   * associated with the given trigger cell.
   */
  std::vector<ldmx::HcalDigiID> contentsOfQuad(
      ldmx::HcalTriggerID triggerCell) const;
  std::vector<ldmx::HcalDigiID> contentsOfSTQ(
      ldmx::HcalTriggerID triggerCell) const;

  /**
   * Returns which trigger cell this precision cell is associated with, or a
   * null id if there is no such association.
   */
  ldmx::HcalTriggerID belongsToQuad(ldmx::HcalDigiID precisionCell) const;
  ldmx::HcalTriggerID belongsToSTQ(ldmx::HcalDigiID precisionCell) const;

 private:
  /** Reference to the Hcal geometry used for trigger geometry information */
  const ldmx::HcalGeometry* hcalGeometry_;
};

}  // namespace hcal

#endif  // HCAL_HCALTRIGGERGEOMETRY_H_
