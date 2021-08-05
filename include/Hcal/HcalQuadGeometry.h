#ifndef HCAL_HCALQUADGEOMETRY_H_
#define HCAL_HCALQUADGEOMETRY_H_

// LDMX
#include "DetDescr/HcalID.h"
#include "DetDescr/HcalDigiID.h"
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
   * @class HcalQuadGeometry
   * @brief defines the relationship between HCal strips and CMB quad trigger primitives and
   * provides geometry information for trigger primitives
   */
  class HcalQuadGeometry : public framework::ConditionsObject {
    public:
    static constexpr const char* CONDITIONS_OBJECT_NAME{"HcalQuadGeometry"};

    /* HcalQuadGeometry(); */
    HcalQuadGeometry(const ldmx::HcalGeometry* hcalGeom = 0);

    /**
     * Returns the set of precision (full-granularity/DAQ) cells which are
     * associated with the given trigger cell.
     */
    std::vector<ldmx::HcalDigiID> contentsOfTriggerCell(
        ldmx::HcalTriggerID triggerCell) const;

    /**
     * Returns which trigger cell this precision cell is associated with, or a
     * null id if there is no such association.
     */
    ldmx::HcalTriggerID belongsTo(ldmx::HcalDigiID precisionCell) const;

 private:
    /** Reference to the Hcal geometry used for trigger geometry information */
    const ldmx::HcalGeometry* hcalGeometry_;
    /** Map of precision cells to trigger cells
     */
    std::map<ldmx::HcalID, ldmx::HcalTriggerID> precision2trigger_;
    /** Map of trigger cells to precision cells
     */
    std::map<ldmx::HcalTriggerID, std::vector<ldmx::HcalID> > trigger2precision_;
  };

}  // namespace hcal

#endif  // HCAL_HCALQUADGEOMETRY_H_
