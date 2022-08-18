/**
 * @file EcalTriggerGeometry.h
 * @brief Class that defines the relationship between precision cells and
 * trigger cells and provides geometry information for trigger cells
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef ECAL_ECALTRIGGERGEOMETRY_H_
#define ECAL_ECALTRIGGERGEOMETRY_H_

// LDMX
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include "Framework/ConditionsObject.h"

// STL
#include <map>
#include <vector>

namespace ldmx {
class EcalGeometry;
}

namespace ecal {

/**
 * @class EcalTriggerGeometry
 * @brief defines the relationship between precision cells and trigger cells and
 * provides geometry information for trigger cells
 */
class EcalTriggerGeometry : public framework::ConditionsObject {
 public:
  static constexpr const char* CONDITIONS_OBJECT_NAME{"EcalTriggerGeometry"};

  EcalTriggerGeometry(int symmetry, const ldmx::EcalGeometry* ecalGeom = 0);

  /**
   * Returns the set of precision (full-granularity/DAQ) cells which are
   * associated with the given trigger cell.
   */
  std::vector<ldmx::EcalID> contentsOfTriggerCell(
      ldmx::EcalTriggerID triggerCell) const;

  /**
   * Returns the set of precision (full-granularity/DAQ) cell which is the
   * center of the given trigger cell.
   */
  ldmx::EcalID centerInTriggerCell(ldmx::EcalTriggerID triggerCell) const;

  /**
   * Returns which trigger cell this precision cell is associated with, or a
   * null id if there is no such association.
   */
  ldmx::EcalTriggerID belongsTo(ldmx::EcalID precisionCell) const;

  /** 
   * Returns the center of the given trigger cell in world coordinates
   *
   * depends on Ecal Geometry (ldmx::EcalGeometry) 
   *
   * C++17's structured bindings is helpful here
   * ```cpp
   * auto [x,y,z] = trig_geom.globalPosition(triggerCell);
   * // x,y,z are the world coordinates of the center of the trigger cell
   * ```
   */
  std::tuple<double, double,double> globalPosition(
      ldmx::EcalTriggerID triggerCell) const;

  /** 
   * Returns the local (within module) center of the given trigger cell
   *
   * depends on Ecal Geometry (ldmx::EcalGeometry) 
   */
  std::pair<double, double> localPosition(
      ldmx::EcalTriggerID triggerCell) const;

 private:
  /** Identifies what symmetries apply in this case, such as all layers being
     identical, or all even and odd planes being identical and whether all
     modules on the same plane are identical.
  */
  int symmetry_;
  /** Reference to the Ecal geometry used for trigger geometry information */
  const ldmx::EcalGeometry* ecalGeometry_;
  /** Map of precision cells to trigger cells, under symmetry assumptions
   */
  std::map<ldmx::EcalID, ldmx::EcalTriggerID> precision2trigger_;
  /** Map of trigger cells to precision cells, under symmetry assumptions
   */
  std::map<ldmx::EcalTriggerID, std::vector<ldmx::EcalID> > trigger2precision_;
};

}  // namespace ecal

#endif  // ECAL_ECALTRIGGERGEOMETRY_H_
