#include "Ecal/EcalTriggerGeometry.h"
#include "Event/EventHeader.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/ConditionsObjectProvider.h"
#include <iostream>
#include <sstream>

namespace ldmx {

  static const int LAYERS_MASK     =0xFF;
  static const int LAYERS_IDENTICAL=0x01;
  static const int LAYERS_ODDEVEN  =0x02;

  static const int MODULES_MASK    =0xFF00;
  static const int INPLANE_IDENTICAL = 0x0100;
  
  
  EcalTriggerGeometry::EcalTriggerGeometry(int symmetry, const EcalHexReadout* ecalGeom) : ConditionsObject(CONDITIONS_OBJECT_NAME), symmetry_{symmetry}, ecalGeometry_{ecalGeom} {
    
    if ((symmetry_ & MODULES_MASK)==INPLANE_IDENTICAL) {
      // first set is the same regardless of alignment...
      int tcell=0;
      /// lower left-sector
      for (int v=1; v<=10; v+=3) {
	for (int u=1; u<=10; u+=3) {
	  EcalTriggerID tid(0,0,tcell);
	  std::vector<EcalID> pids;
	  for (int du=-1; du<=1; du++) {
	    for (int dv=-1; dv<=1; dv++) {
	      EcalID pid(0,0,u+du, v+dv);
	      precision2trigger_[pid]=tid;
	      pids.push_back(pid);
	    }
	  }
	  trigger2precision_[tid]=pids;
	  tcell++;
	}
      }
      /// upper-left sector
      for (int v=13; v<=22; v+=3) {
	for (int u=v-10; u<=v; u+=3) {
	  EcalTriggerID tid(0,0,tcell);
	  std::vector<EcalID> pids;
	  for (int dv=-1; dv<=1; dv++) {
	    for (int du=-1; du<=1; du++) {
	      EcalID pid(0,0,u+du+dv,v+dv); // changes directions here
	      precision2trigger_[pid]=tid;
	      pids.push_back(pid);
	    }
	  }
	  trigger2precision_[tid]=pids;
	  tcell++;
	}
      }
      // right side
      for (int v=2; v<=22; v+=3) {
	int irow=(v-2)/3;
	for (int icol=0; icol<=std::min(irow,3); icol++) {
	  if (irow-icol>=4) continue;
	  EcalTriggerID tid(0,0,tcell);
	  std::vector<EcalID> pids;
	  int u=13+3*icol;
	  for (int dv=-1; dv<=1; dv++) {
	    for (int du=-1; du<=1; du++) {	
	      EcalID pid(0,0,u+du,v+du+dv);
	      precision2trigger_[pid]=tid;	  
	      pids.push_back(pid);
	    }
	  }
	  trigger2precision_[tid]=pids;
	  tcell++;
	}
      }

    } else {
      // raise an exception...
    }
    
  }

  std::vector<EcalID> EcalTriggerGeometry::contentsOfTriggerCell(EcalTriggerID triggerCell) const {
    EcalTriggerID effId;
    std::vector<EcalID> retval;
    if ((symmetry_ & MODULES_MASK)==INPLANE_IDENTICAL) {
      effId=EcalTriggerID(0,0,triggerCell.triggercell());  
    }
    auto ptr=trigger2precision_.find(effId);
    if (ptr!=trigger2precision_.end()) {
      for (auto idz : ptr->second) {
	retval.push_back(EcalID(triggerCell.layer(),triggerCell.module(),idz.cell()));
      }
    }
    return retval;
  }

  EcalID EcalTriggerGeometry::centerInTriggerCell(EcalTriggerID triggerCell) const {
    EcalTriggerID effId;
    if ((symmetry_ & MODULES_MASK)==INPLANE_IDENTICAL) {
      effId=EcalTriggerID(0,0,triggerCell.triggercell());  
    }
    auto ptr=trigger2precision_.find(effId);
    if (ptr==trigger2precision_.end()) {
      std::stringstream ss;
      ss << "Unable to find trigger cell " << triggerCell;
      EXCEPTION_RAISE("EcalGeometryException",ss.str());
    }

    return EcalID(triggerCell.layer(),triggerCell.module(),ptr->second[4].cell());
  }

  
  EcalTriggerID EcalTriggerGeometry::belongsTo(EcalID precisionCell) const {
    EcalID effId;
    if ((symmetry_ & MODULES_MASK)==INPLANE_IDENTICAL) {
      effId=EcalID(0,0,precisionCell.cell());  
    }
    auto ptr=precision2trigger_.find(effId);
    if (ptr==precision2trigger_.end()) {
      return EcalTriggerID(0,0,0); // not ideal
    } else {
      return EcalTriggerID(precisionCell.layer(),precisionCell.module(),ptr->second.triggercell());
    }
  }


  // as it happens, the fifth precision cell in the list is the center cell
  std::pair<double,double> EcalTriggerGeometry::globalPosition(EcalTriggerID triggerCell) const {
    if (!ecalGeometry_) return std::pair<double,double>(0,0);
    EcalID pid=centerInTriggerCell(triggerCell);
    return ecalGeometry_->getCellCenterAbsolute(pid);
  }

  std::pair<double,double> EcalTriggerGeometry::localPosition(EcalTriggerID triggerCell) const {
      if (!ecalGeometry_) return std::pair<double,double>(0,0);
      EcalID pid=centerInTriggerCell(triggerCell);
      return ecalGeometry_->getCellCenterRelative(pid.cell());
  }

class EcalTriggerGeometryProvider : public ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   */	
  EcalTriggerGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) :
      ConditionsObjectProvider(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME, tagname, parameters, process), ecalTriggerGeometry_{nullptr} {
  }


  /** Destructor */
  virtual ~EcalTriggerGeometryProvider() {
    if (ecalTriggerGeometry_!=nullptr) delete ecalTriggerGeometry_;
    ecalTriggerGeometry_=nullptr;
  }
	
  /**
   * Provides access to the EcalHexReadout or EcalTriggerGeometry
   * @note Currently, these are assumed to be valid for all time, but this behavior could be changed.  Users should not cache the pointer
   * between events
   */
  virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const EventHeader& context) {
    if (ecalTriggerGeometry_==nullptr) {
      std::pair<const ConditionsObject*,ConditionsIOV> cond_ecal_geom=requestParentCondition(EcalHexReadout::CONDITIONS_OBJECT_NAME, context);
      const EcalHexReadout* ecalgeom=dynamic_cast<const EcalHexReadout*>(cond_ecal_geom.first);
      ecalTriggerGeometry_=new EcalTriggerGeometry(INPLANE_IDENTICAL | LAYERS_IDENTICAL, ecalgeom);
    }
   return std::make_pair(ecalTriggerGeometry_,ConditionsIOV(context.getRun(),context.getRun(),true,true)); 
  }

  /**
   * Take no action on release, as the object is permanently owned by the Provider
   */
  virtual void releaseConditionsObject(const ConditionsObject* co) {
  }

 private:

  EcalTriggerGeometry* ecalTriggerGeometry_;
};

}
DECLARE_CONDITIONS_PROVIDER_NS(ldmx, EcalTriggerGeometryProvider);
