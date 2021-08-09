#include "Hcal/HcalTriggerGeometry.h"
#include <iostream>
#include <sstream>
#include "DetDescr/HcalGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

/*
  belongToQuad
  belongToSTQ
  just use simple functions.. should be easier than defining a mapping
  ..
  then in the tp producer write out 2 copies of maps <int, int> = <id, charge>
  fot STQs simply use a linear scale output
 */

namespace hcal {

HcalTriggerGeometry::HcalTriggerGeometry(const ldmx::HcalGeometry* hcalGeom)
  : ConditionsObject(CONDITIONS_OBJECT_NAME),
    hcalGeometry_{hcalGeom} {

  // // specify CMB quads for back HCal
  // const int nStrips = hcalGeometry_->getNumStrips(ldmx::HcalID::HcalSection::BACK);
  // for (int strip=0; strip < nStrips; strip++){
  //   int sstrip = strip/4;
  //   ldmx::HcalTriggerID tid(ldmx::HcalID::HcalSection::BACK, 0, sstrip, 0);
  //   ldmx::HcalID pid(ldmx::HcalID::HcalSection::BACK, 0, strip);
  //   precision2trigger_[pid] = tid;
  //   if (strip%4==0) {
  //     trigger2precision_[tid] = std::vector<ldmx::HcalID>({pid});
  //   } else {
  //     trigger2precision_[tid].push_back(pid);
  //   }
  //   // for superstrips, layer also matters
  //   const int nLayers = hcalGeometry_->getNumLayers(ldmx::HcalID::HcalSection::BACK);
  //   for (int layer=0; layer < nLayers; layer++){
  //     int superlayer = 2*(layer / 4) + (layer % 4)%2; // 0 1 0 1 2 3 2 3 
  //     if (strip%8==0 && (layer%4<3)) {
  //       Super2precision_[tid] = std::vector<ldmx::HcalID>({pid});
  //     } else {
  //       super2precision_[tid].push_back(pid);
  //     }
  //   // if (strip%8==0) {
  //   //   super2trigger_[tid] = std::vector<ldmx::HcalTriggerID>({pid});
  //   // }

  //   // if (strip%8==0) super2trigger_[tid] = std::vector<ldmx::HcalTriggerID>({pid});
  //   // if (strip%4==0) trigger2precision_[tid] = std::vector<ldmx::HcalID>({pid});
      
    
  // }

  // // for side HCal
  // const int nStripsSide = hcalGeometry_->getNumStrips(ldmx::HcalID::HcalSection::TOP);
  // for (int strip=0; strip < nStripsSide; strip++){
  //   int sstrip = strip/4;
  //   ldmx::HcalTriggerID tid(ldmx::HcalID::HcalSection::TOP, 0, sstrip, 0);
  //   ldmx::HcalID pid(ldmx::HcalID::HcalSection::TOP, 0, strip);
  //   precision2trigger_[pid] = tid;
  //   if (strip%4==0) trigger2precision_[tid] = std::vector<ldmx::HcalID>({pid});
  //   else trigger2precision_[tid].push_back(pid);
  // }

  // // group multiple trigger cells into supercells
  
}

// // std::vector<ldmx::HcalDigiID> HcalQuadGeometry::contentsOfTriggerCell(
// //     ldmx::HcalTriggerID triggerCell) const {
// std::vector<ldmx::HcalDigiID> HcalQuadGeometry::contentsOfQuad(
//     ldmx::HcalTriggerID triggerCell) const {
//   const int effSection = (triggerCell.section() == ldmx::HcalID::HcalSection::BACK ?
//                        ldmx::HcalID::HcalSection::BACK :
//                        ldmx::HcalID::HcalSection::TOP);
//   ldmx::HcalTriggerID effId(effSection, 0, triggerCell.superstrip(), 0);
//   std::vector<ldmx::HcalDigiID> retval;
//   auto ptr = trigger2precision_.find(effId);
//   if (ptr != trigger2precision_.end()) {
//     for (auto idz : ptr->second) {
//       retval.push_back(
//       ldmx::HcalDigiID(triggerCell.section(), triggerCell.layer(), idz.strip(), triggerCell.end()));
//     }
//   }
//   return retval;
// }

// std::vector<ldmx::HcalDigiID> HcalQuadGeometry::contentsOfSTQ(
//     ldmx::HcalTriggerID triggerCell) const {
//   const int effSection = (triggerCell.section() == ldmx::HcalID::HcalSection::BACK ?
//                        ldmx::HcalID::HcalSection::BACK :
//                        ldmx::HcalID::HcalSection::TOP);
//   ldmx::HcalTriggerID effId(effSection, 0, triggerCell.superstrip(), 0);
//   std::vector<ldmx::HcalDigiID> retval;
//   auto ptr = trigger2precision_.find(effId);
//   if (ptr != trigger2precision_.end()) {
//     for (auto idz : ptr->second) {
//       retval.push_back(
//       ldmx::HcalDigiID(triggerCell.section(), triggerCell.layer(), idz.strip(), triggerCell.end()));
//     }
//   }
//   return retval;
// }
  
// ldmx::HcalTriggerID HcalQuadGeometry::belongsToSTC(
//     ldmx::HcalDigiID precisionCell) const {
// }
// ldmx::HcalTriggerID HcalQuadGeometry::belongsTo(


std::vector<ldmx::HcalDigiID> HcalTriggerGeometry::contentsOfQuad(
    ldmx::HcalTriggerID triggerCell) const {
  std::vector<ldmx::HcalDigiID> retval;
  for(int iStrip=0;iStrip<4;iStrip++){
    retval.push_back( ldmx::HcalDigiID(triggerCell.section(), triggerCell.layer(),
                                       iStrip+4*triggerCell.superstrip(), triggerCell.end()) );
    
  }
  return retval;
}
std::vector<ldmx::HcalDigiID> HcalTriggerGeometry::contentsOfSTQ(
    ldmx::HcalTriggerID triggerCell) const {
  std::vector<ldmx::HcalDigiID> retval;
  for(int iStrip=0;iStrip<8;iStrip++){
    for(int iLayer=0;iLayer<2;iLayer++){
      int tlayer = triggerCell.layer();
      int player = 2*(tlayer+iStrip) - tlayer%2;
      retval.push_back(ldmx::HcalDigiID(triggerCell.section(), player,
                                        iStrip+8*triggerCell.superstrip(), triggerCell.end()));
    }
  }
  return retval;
}
  
ldmx::HcalTriggerID HcalTriggerGeometry::belongsToQuad(
    ldmx::HcalDigiID precisionCell) const {
  return ldmx::HcalTriggerID(precisionCell.section(), precisionCell.layer(),
                             precisionCell.strip()/4, precisionCell.end());
}

ldmx::HcalTriggerID HcalTriggerGeometry::belongsToSTQ(
    ldmx::HcalDigiID precisionCell) const {
  // 2x2 groups of quads, with "superlayers" like:
  // 0 1 0 1 2 3 2 3 ...
  // V H V H V H V H ...
  int layer = precisionCell.layer();
  int superlayer = 2*(layer / 4) + (layer % 4)%2;
  return ldmx::HcalTriggerID(precisionCell.section(), superlayer,
                             precisionCell.strip()/8, precisionCell.end());
}

//   const int effSection = (precisionCell.section() == ldmx::HcalID::HcalSection::BACK ?
//                        ldmx::HcalID::HcalSection::BACK :
//                        ldmx::HcalID::HcalSection::TOP);
  
//   ldmx::HcalID effId(effSection, 0, precisionCell.strip());
//   auto ptr = precision2trigger_.find(effId);
//   if (ptr != precision2trigger_.end()) {
//     return ldmx::HcalTriggerID(precisionCell.section(), precisionCell.layer(),
//                                ptr->second.superstrip(), precisionCell.end());
//   }
//   return ldmx::HcalTriggerID();
// }

class HcalTriggerGeometryProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   */
  HcalTriggerGeometryProvider(const std::string& name,
                              const std::string& tagname,
                              const framework::config::Parameters& parameters,
                              framework::Process& process)
      : ConditionsObjectProvider(HcalTriggerGeometry::CONDITIONS_OBJECT_NAME,
                                 tagname, parameters, process),
        hcalTriggerGeometry_{nullptr} {}

  /** Destructor */
  virtual ~HcalTriggerGeometryProvider() {
    if (hcalTriggerGeometry_ != nullptr) delete hcalTriggerGeometry_;
    hcalTriggerGeometry_ = nullptr;
  }

  /**
   * Provides access to the HcalGeometry or HcalTriggerGeometry
   * @note Currently, these are assumed to be valid for all time, but this
   * behavior could be changed.  Users should not cache the pointer between
   * events
   */
  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) {
    if (hcalTriggerGeometry_ == nullptr) {
      std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
          cond_hcal_geom = requestParentCondition(
              ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME, context);
      const ldmx::HcalGeometry* hcalgeom =
          dynamic_cast<const ldmx::HcalGeometry*>(cond_hcal_geom.first);
      hcalTriggerGeometry_ = new HcalTriggerGeometry(hcalgeom);
    }
    return std::make_pair(hcalTriggerGeometry_,
                          framework::ConditionsIOV(
                              context.getRun(), context.getRun(), true, true));
  }

  /**
   * Take no action on release, as the object is permanently owned by the
   * Provider
   */
  virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

 private:
  HcalTriggerGeometry* hcalTriggerGeometry_;
};

}  // namespace hcal
DECLARE_CONDITIONS_PROVIDER_NS(hcal, HcalTriggerGeometryProvider);
