/**
 * @file EcalTriggerGeometry.h
 * @brief Class that defines the relationship between precision cells and trigger cells and provides geometry information for trigger cells
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef ECAL_ECALTRIGGERGEOMETRY_H_
#define ECAL_ECALTRIGGERGEOMETRY_H_

// LDMX
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include "Framework/ConditionsObject.h"

// STL
#include <vector>
#include <map>

namespace ldmx {

  /**
   * @class EcalTriggerGeometry
   * @brief defines the relationship between precision cells and trigger cells and provides geometry information for trigger cells
   */
  class EcalTriggerGeometry : public ConditionsObject {
  public:
    EcalTriggerGeometry(int symmetry);
    
    /**
     * Returns the set of precision (full-granularity/DAQ) cells which are associated with the given trigger cell.
     */
    std::vector<EcalID> contentsOfTriggerCell(EcalTriggerID triggerCell) const;

    /**
     * Returns which trigger cell this precision cell is associated with, or a null id if there is no such association.
     */
    EcalTriggerID belongsTo(EcalID precisionCell) const;
  private:
    /** Identifies what symmetries apply in this case, such as all layers being identical, or 
	all even and odd planes being identical and whether all modules on the same plane are identical.
    */
    int symmetry_;
    /** Map of precision cells to trigger cells, under symmetry assumptions
     */
    std::map<EcalID,EcalTriggerID> precision2trigger_;
    /** Map of trigger cells to precision cells, under symmetry assumptions
     */
    std::map<EcalTriggerID,std::vector<EcalID> > trigger2precision_;
  };
  
}


#endif // ECAL_ECALTRIGGERGEOMETRY_H_
