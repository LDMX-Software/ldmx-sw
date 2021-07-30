#ifndef HCAL_HCALTRIGGERGEOMETRY_H_
#define HCAL_HCALTRIGGERGEOMETRY_H_

// LDMX
#include "DetDescr/HcalID.h"
#include "DetDescr/HcalTriggerID.h"
#include "Framework/ConditionsObject.h"

// STL
#include <map>
#include <vector>

/* namespace ldmx { */
/*   class HcalHexReadout; // TODO */
/* } */

namespace hcal {

  /**
   * @class HcalTriggerGeometry
   * @brief defines the relationship between precision cells and trigger cells and
   * provides geometry information for trigger cells
   */
  class HcalTriggerGeometry : public framework::ConditionsObject {
 public:
    static constexpr const char* CONDITIONS_OBJECT_NAME{"HcalTriggerGeometry"};

    HcalTriggerGeometry();
    //HcalTriggerGeometry(int symmetry, const ldmx::HcalHexReadout* hcalGeom = 0);

    /**
     * Returns the set of precision (full-granularity/DAQ) cells which are
     * associated with the given trigger cell.
     */
    std::vector<ldmx::HcalID> contentsOfTriggerCell(
        ldmx::HcalTriggerID triggerCell) const;

    /**
     * Returns the set of precision (full-granularity/DAQ) cell which is the
     * center of the given trigger cell.
     */
    ldmx::HcalID centerInTriggerCell(ldmx::HcalTriggerID triggerCell) const;

    /**
     * Returns which trigger cell this precision cell is associated with, or a
     * null id if there is no such association.
     */
    ldmx::HcalTriggerID belongsTo(ldmx::HcalID precisionCell) const;

    /** Returns the center of the given trigger cell, depends on Hcal Geometry
     * (ldmx::HcalHexReadout) */ //TODO
    std::pair<double, double> globalPosition(
        ldmx::HcalTriggerID triggerCell) const;

    /** Returns the local (within module) center of the given trigger cell,
     * depends on Hcal Geometry (ldmx::HcalHexReadout) */ //TODO
    std::pair<double, double> localPosition(
        ldmx::HcalTriggerID triggerCell) const;

 private:
    /** Reference to the Hcal geometry used for trigger geometry information */
    //const ldmx::HcalHexReadout* hcalGeometry_; //TODO
    /** Map of precision cells to trigger cells
     */
    std::map<ldmx::HcalID, ldmx::HcalTriggerID> precision2trigger_;
    /** Map of trigger cells to precision cells
     */
    std::map<ldmx::HcalTriggerID, std::vector<ldmx::HcalID> > trigger2precision_;
  };

}  // namespace hcal

#endif  // HCAL_HCALTRIGGERGEOMETRY_H_
