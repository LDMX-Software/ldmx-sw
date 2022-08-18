#include "Ecal/EcalTriggerGeometry.h"
#include <iostream>
#include <sstream>
#include "DetDescr/EcalGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace ecal {

static const int LAYERS_MASK = 0xFF;
static const int LAYERS_IDENTICAL = 0x01;
static const int LAYERS_ODDEVEN = 0x02;

static const int MODULES_MASK = 0xFF00;
static const int INPLANE_IDENTICAL = 0x0100;

EcalTriggerGeometry::EcalTriggerGeometry(int symmetry,
                                         const ldmx::EcalGeometry* ecalGeom)
    : ConditionsObject(CONDITIONS_OBJECT_NAME),
      symmetry_{symmetry},
      ecalGeometry_{ecalGeom} {
  if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
    // first set is the same regardless of alignment...
    int tcell = 0;
    /// lower left-sector
    for (int v = 1; v <= 10; v += 3) {
      for (int u = 1; u <= 10; u += 3) {
        ldmx::EcalTriggerID tid(0, 0, tcell);
        std::vector<ldmx::EcalID> pids;
        for (int du = -1; du <= 1; du++) {
          for (int dv = -1; dv <= 1; dv++) {
            ldmx::EcalID pid(0, 0, u + du, v + dv);
            precision2trigger_[pid] = tid;
            pids.push_back(pid);
          }
        }
        trigger2precision_[tid] = pids;
        tcell++;
      }
    }
    /// upper-left sector
    for (int v = 13; v <= 22; v += 3) {
      for (int u = v - 10; u <= v; u += 3) {
        ldmx::EcalTriggerID tid(0, 0, tcell);
        std::vector<ldmx::EcalID> pids;
        for (int dv = -1; dv <= 1; dv++) {
          for (int du = -1; du <= 1; du++) {
            ldmx::EcalID pid(0, 0, u + du + dv,
                             v + dv);  // changes directions here
            precision2trigger_[pid] = tid;
            pids.push_back(pid);
          }
        }
        trigger2precision_[tid] = pids;
        tcell++;
      }
    }
    // right side
    for (int v = 2; v <= 22; v += 3) {
      int irow = (v - 2) / 3;
      for (int icol = 0; icol <= std::min(irow, 3); icol++) {
        if (irow - icol >= 4) continue;
        ldmx::EcalTriggerID tid(0, 0, tcell);
        std::vector<ldmx::EcalID> pids;
        int u = 13 + 3 * icol;
        for (int dv = -1; dv <= 1; dv++) {
          for (int du = -1; du <= 1; du++) {
            ldmx::EcalID pid(0, 0, u + du, v + du + dv);
            precision2trigger_[pid] = tid;
            pids.push_back(pid);
          }
        }
        trigger2precision_[tid] = pids;
        tcell++;
      }
    }

  } else {
    // raise an exception...
  }
}

std::vector<ldmx::EcalID> EcalTriggerGeometry::contentsOfTriggerCell(
    ldmx::EcalTriggerID triggerCell) const {
  ldmx::EcalTriggerID effId;
  std::vector<ldmx::EcalID> retval;
  if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
    effId = ldmx::EcalTriggerID(0, 0, triggerCell.triggercell());
  }
  auto ptr = trigger2precision_.find(effId);
  if (ptr != trigger2precision_.end()) {
    for (auto idz : ptr->second) {
      retval.push_back(
          ldmx::EcalID(triggerCell.layer(), triggerCell.module(), idz.cell()));
    }
  }
  return retval;
}

ldmx::EcalID EcalTriggerGeometry::centerInTriggerCell(
    ldmx::EcalTriggerID triggerCell) const {
  ldmx::EcalTriggerID effId;
  if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
    effId = ldmx::EcalTriggerID(0, 0, triggerCell.triggercell());
  }
  auto ptr = trigger2precision_.find(effId);
  if (ptr == trigger2precision_.end()) {
    std::stringstream ss;
    ss << "Unable to find trigger cell " << triggerCell;
    EXCEPTION_RAISE("EcalGeometryException", ss.str());
  }

  return ldmx::EcalID(triggerCell.layer(), triggerCell.module(),
                      ptr->second[4].cell());
}

ldmx::EcalTriggerID EcalTriggerGeometry::belongsTo(
    ldmx::EcalID precisionCell) const {
  ldmx::EcalID effId;
  if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
    effId = ldmx::EcalID(0, 0, precisionCell.cell());
  }
  auto ptr = precision2trigger_.find(effId);
  if (ptr == precision2trigger_.end()) {
    return ldmx::EcalTriggerID(0, 0, 0);  // not ideal
  } else {
    return ldmx::EcalTriggerID(precisionCell.layer(), precisionCell.module(),
                               ptr->second.triggercell());
  }
}

// as it happens, the fifth precision cell in the list is the center cell
std::tuple<double, double, double> EcalTriggerGeometry::globalPosition(
    ldmx::EcalTriggerID triggerCell) const {
  if (!ecalGeometry_) return std::make_tuple(0,0,0);
  ldmx::EcalID pid = centerInTriggerCell(triggerCell);
  return ecalGeometry_->getPosition(pid);
}

std::pair<double, double> EcalTriggerGeometry::localPosition(
    ldmx::EcalTriggerID triggerCell) const {
  if (!ecalGeometry_) return std::make_pair(0,0);
  ldmx::EcalID pid = centerInTriggerCell(triggerCell);
  return ecalGeometry_->getPositionInModule(pid.cell());
}

class EcalTriggerGeometryProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   */
  EcalTriggerGeometryProvider(const std::string& name,
                              const std::string& tagname,
                              const framework::config::Parameters& parameters,
                              framework::Process& process)
      : ConditionsObjectProvider(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME,
                                 tagname, parameters, process),
        ecalTriggerGeometry_{nullptr} {}

  /** Destructor */
  virtual ~EcalTriggerGeometryProvider() {
    if (ecalTriggerGeometry_ != nullptr) delete ecalTriggerGeometry_;
    ecalTriggerGeometry_ = nullptr;
  }

  /**
   * Provides access to the EcalGeometry or EcalTriggerGeometry
   * @note Currently, these are assumed to be valid for all time, but this
   * behavior could be changed.  Users should not cache the pointer between
   * events
   */
  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) {
    if (ecalTriggerGeometry_ == nullptr) {
      std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
          cond_ecal_geom = requestParentCondition(
              ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME, context);
      const ldmx::EcalGeometry* ecalgeom =
          dynamic_cast<const ldmx::EcalGeometry*>(cond_ecal_geom.first);
      ecalTriggerGeometry_ = new EcalTriggerGeometry(
          INPLANE_IDENTICAL | LAYERS_IDENTICAL, ecalgeom);
    }
    return std::make_pair(ecalTriggerGeometry_,
                          framework::ConditionsIOV(
                              context.getRun(), context.getRun(), true, true));
  }

  /**
   * Take no action on release, as the object is permanently owned by the
   * Provider
   */
  virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

 private:
  EcalTriggerGeometry* ecalTriggerGeometry_;
};

}  // namespace ecal
DECLARE_CONDITIONS_PROVIDER_NS(ecal, EcalTriggerGeometryProvider);
