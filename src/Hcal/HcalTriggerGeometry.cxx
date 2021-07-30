#include "Hcal/HcalTriggerGeometry.h"
#include <iostream>
#include <sstream>
//#include "DetDescr/HcalHexReadout.h"
#include "DetDescr/HcalGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace hcal {

// static const int LAYERS_MASK = 0xFF;
// static const int LAYERS_IDENTICAL = 0x01;
// static const int LAYERS_ODDEVEN = 0x02;

// static const int MODULES_MASK = 0xFF00;
// static const int INPLANE_IDENTICAL = 0x0100;

// HcalTriggerGeometry::HcalTriggerGeometry(int symmetry,
//                                          const ldmx::HcalHexReadout* hcalGeom)
  //   : ConditionsObject(CONDITIONS_OBJECT_NAME)
  // hcalGeometry_{hcalGeom} {
HcalTriggerGeometry::HcalTriggerGeometry()
    : ConditionsObject(CONDITIONS_OBJECT_NAME) {
    // if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
    //   // first set is the same regardless of alignment...
    //   int tcell = 0;
    //   /// lower left-sector
    //   for (int v = 1; v <= 10; v += 3) {
    //     for (int u = 1; u <= 10; u += 3) {
    //       ldmx::HcalTriggerID tid(0, 0, tcell);
    //       std::vector<ldmx::HcalID> pids;
    //       for (int du = -1; du <= 1; du++) {
    //         for (int dv = -1; dv <= 1; dv++) {
    //           ldmx::HcalID pid(0, 0, u + du, v + dv);
    //           precision2trigger_[pid] = tid;
    //           pids.push_back(pid);
    //         }
    //       }
    //       trigger2precision_[tid] = pids;
    //       tcell++;
    //     }
    //   }
    //   /// upper-left sector
    //   for (int v = 13; v <= 22; v += 3) {
    //     for (int u = v - 10; u <= v; u += 3) {
    //       ldmx::HcalTriggerID tid(0, 0, tcell);
    //       std::vector<ldmx::HcalID> pids;
    //       for (int dv = -1; dv <= 1; dv++) {
    //         for (int du = -1; du <= 1; du++) {
    //           ldmx::HcalID pid(0, 0, u + du + dv,
    //                            v + dv);  // changes directions here
    //           precision2trigger_[pid] = tid;
    //           pids.push_back(pid);
    //         }
    //       }
    //       trigger2precision_[tid] = pids;
    //       tcell++;
    //     }
    //   }
    //   // right side
    //   for (int v = 2; v <= 22; v += 3) {
    //     int irow = (v - 2) / 3;
    //     for (int icol = 0; icol <= std::min(irow, 3); icol++) {
    //       if (irow - icol >= 4) continue;
    //       ldmx::HcalTriggerID tid(0, 0, tcell);
    //       std::vector<ldmx::HcalID> pids;
    //       int u = 13 + 3 * icol;
    //       for (int dv = -1; dv <= 1; dv++) {
    //         for (int du = -1; du <= 1; du++) {
    //           ldmx::HcalID pid(0, 0, u + du, v + du + dv);
    //           precision2trigger_[pid] = tid;
    //           pids.push_back(pid);
    //         }
    //       }
    //       trigger2precision_[tid] = pids;
    //       tcell++;
    //     }
    //   }

    // } else {
    //   // raise an exception...
    // }
}

std::vector<ldmx::HcalID> HcalTriggerGeometry::contentsOfTriggerCell(
    ldmx::HcalTriggerID triggerCell) const {
  ldmx::HcalTriggerID effId;
  std::vector<ldmx::HcalID> retval;
  // if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
  //   effId = ldmx::HcalTriggerID(0, 0, triggerCell.triggercell());
  // }
  // auto ptr = trigger2precision_.find(effId);
  // if (ptr != trigger2precision_.end()) {
  //   for (auto idz : ptr->second) {
  //     retval.push_back(
  //         ldmx::HcalID(triggerCell.layer(), triggerCell.module(), idz.cell()));
  //   }
  // }
  return retval;
}

ldmx::HcalID HcalTriggerGeometry::centerInTriggerCell(
    ldmx::HcalTriggerID triggerCell) const {
  // ldmx::HcalTriggerID effId;
  // if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
  //   effId = ldmx::HcalTriggerID(0, 0, triggerCell.triggercell());
  // }
  // auto ptr = trigger2precision_.find(effId);
  // if (ptr == trigger2precision_.end()) {
  //   std::stringstream ss;
  //   ss << "Unable to find trigger cell " << triggerCell;
  //   EXCEPTION_RAISE("HcalGeometryException", ss.str());
  // }

  // return ldmx::HcalID(triggerCell.layer(), triggerCell.module(),
  //                     ptr->second[4].cell());
    return ldmx::HcalID();
}

ldmx::HcalTriggerID HcalTriggerGeometry::belongsTo(
    ldmx::HcalID precisionCell) const {
  // ldmx::HcalID effId;
  // if ((symmetry_ & MODULES_MASK) == INPLANE_IDENTICAL) {
  //   effId = ldmx::HcalID(0, 0, precisionCell.cell());
  // }
  // auto ptr = precision2trigger_.find(effId);
  // if (ptr == precision2trigger_.end()) {
  //   return ldmx::HcalTriggerID(0, 0, 0);  // not ideal
  // } else {
  //   return ldmx::HcalTriggerID(precisionCell.layer(), precisionCell.module(),
  //                              ptr->second.triggercell());
  // }
  return ldmx::HcalTriggerID();
}

// as it happens, the fifth precision cell in the list is the center cell
std::pair<double, double> HcalTriggerGeometry::globalPosition(
    ldmx::HcalTriggerID triggerCell) const {
  // if (!hcalGeometry_) return std::pair<double, double>(0, 0);
  // ldmx::HcalID pid = centerInTriggerCell(triggerCell);
  // return hcalGeometry_->getCellCenterAbsolute(pid);
  return std::pair<double, double>(0, 0);
}

std::pair<double, double> HcalTriggerGeometry::localPosition(
    ldmx::HcalTriggerID triggerCell) const {
  // if (!hcalGeometry_) return std::pair<double, double>(0, 0);
  // ldmx::HcalID pid = centerInTriggerCell(triggerCell);
  // return hcalGeometry_->getCellCenterRelative(pid.cell());
  return std::pair<double, double>(0, 0);
}

// class HcalTriggerGeometryProvider : public framework::ConditionsObjectProvider {
//  public:
//   /**
//    * Class constructor
//    */
//   HcalTriggerGeometryProvider(const std::string& name,
//                               const std::string& tagname,
//                               const framework::config::Parameters& parameters,
//                               framework::Process& process)
//       : ConditionsObjectProvider(HcalTriggerGeometry::CONDITIONS_OBJECT_NAME,
//                                  tagname, parameters, process),
//         hcalTriggerGeometry_{nullptr} {}

//   /** Destructor */
//   virtual ~HcalTriggerGeometryProvider() {
//     if (hcalTriggerGeometry_ != nullptr) delete hcalTriggerGeometry_;
//     hcalTriggerGeometry_ = nullptr;
//   }

//   /**
//    * Provides access to the HcalHexReadout or HcalTriggerGeometry
//    * @note Currently, these are assumed to be valid for all time, but this
//    * behavior could be changed.  Users should not cache the pointer between
//    * events
//    */
//   virtual std::pair<const framework::ConditionsObject*,
//                     framework::ConditionsIOV>
//   getCondition(const ldmx::EventHeader& context) {
//     if (hcalTriggerGeometry_ == nullptr) {
//       std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
//           cond_hcal_geom = requestParentCondition(
//               ldmx::HcalHexReadout::CONDITIONS_OBJECT_NAME, context);
//       const ldmx::HcalHexReadout* hcalgeom =
//           dynamic_cast<const ldmx::HcalHexReadout*>(cond_hcal_geom.first);
//       hcalTriggerGeometry_ = new HcalTriggerGeometry(
//           INPLANE_IDENTICAL | LAYERS_IDENTICAL, hcalgeom);
//     }
//     return std::make_pair(hcalTriggerGeometry_,
//                           framework::ConditionsIOV(
//                               context.getRun(), context.getRun(), true, true));
//   }

//   /**
//    * Take no action on release, as the object is permanently owned by the
//    * Provider
//    */
//   virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

//  private:
//   HcalTriggerGeometry* hcalTriggerGeometry_;
// };

}  // namespace hcal
//DECLARE_CONDITIONS_PROVIDER_NS(hcal, HcalTriggerGeometryProvider);
