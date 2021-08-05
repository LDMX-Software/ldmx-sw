#include "Hcal/HcalQuadGeometry.h"
#include <iostream>
#include <sstream>
#include "DetDescr/HcalGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace hcal {

HcalQuadGeometry::HcalQuadGeometry(const ldmx::HcalGeometry* hcalGeom)
  : ConditionsObject(CONDITIONS_OBJECT_NAME),
    hcalGeometry_{hcalGeom} {

  // specify CMB quads for back HCal
  const int nStrips = hcalGeometry_->getNumStrips(ldmx::HcalID::HcalSection::BACK);
  for (int strip=0; strip < nStrips; strip++){
    int sstrip = strip/4;
    ldmx::HcalTriggerID tid(ldmx::HcalID::HcalSection::BACK, 0, sstrip, 0);
    ldmx::HcalID pid(ldmx::HcalID::HcalSection::BACK, 0, strip);
    precision2trigger_[pid] = tid;
    if (strip%4==0) trigger2precision_[tid] = std::vector<ldmx::HcalID>({pid});
    else trigger2precision_[tid].push_back(pid);    
  }

  // for side HCal
  const int nStripsSide = hcalGeometry_->getNumStrips(ldmx::HcalID::HcalSection::TOP);
  for (int strip=0; strip < nStripsSide; strip++){
    int sstrip = strip/4;
    ldmx::HcalTriggerID tid(ldmx::HcalID::HcalSection::TOP, 0, sstrip, 0);
    ldmx::HcalID pid(ldmx::HcalID::HcalSection::TOP, 0, strip);
    precision2trigger_[pid] = tid;
    if (strip%4==0) trigger2precision_[tid] = std::vector<ldmx::HcalID>({pid});
    else trigger2precision_[tid].push_back(pid);
  }
}

std::vector<ldmx::HcalDigiID> HcalQuadGeometry::contentsOfTriggerCell(
    ldmx::HcalTriggerID triggerCell) const {
  const int effSection = (triggerCell.section() == ldmx::HcalID::HcalSection::BACK ?
                       ldmx::HcalID::HcalSection::BACK :
                       ldmx::HcalID::HcalSection::TOP);
  ldmx::HcalTriggerID effId(effSection, 0, triggerCell.superstrip(), 0);
  std::vector<ldmx::HcalDigiID> retval;
  auto ptr = trigger2precision_.find(effId);
  if (ptr != trigger2precision_.end()) {
    for (auto idz : ptr->second) {
      retval.push_back(
      ldmx::HcalDigiID(triggerCell.section(), triggerCell.layer(), idz.strip(), triggerCell.end()));
    }
  }
  return retval;
}

ldmx::HcalTriggerID HcalQuadGeometry::belongsTo(
    ldmx::HcalDigiID precisionCell) const {
  const int effSection = (precisionCell.section() == ldmx::HcalID::HcalSection::BACK ?
                       ldmx::HcalID::HcalSection::BACK :
                       ldmx::HcalID::HcalSection::TOP);

  ldmx::HcalDigiID effId(effSection, 0, precisionCell.strip(), 0);
  auto ptr = precision2trigger_.find(effId);
  if (ptr != precision2trigger_.end()) {
    return ldmx::HcalTriggerID(precisionCell.section(), precisionCell.layer(),
                               ptr->second.superstrip(), precisionCell.end());
  }
  return ldmx::HcalTriggerID();
}

class HcalQuadGeometryProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   */
  HcalQuadGeometryProvider(const std::string& name,
                              const std::string& tagname,
                              const framework::config::Parameters& parameters,
                              framework::Process& process)
      : ConditionsObjectProvider(HcalQuadGeometry::CONDITIONS_OBJECT_NAME,
                                 tagname, parameters, process),
        hcalTriggerGeometry_{nullptr} {}

  /** Destructor */
  virtual ~HcalQuadGeometryProvider() {
    if (hcalTriggerGeometry_ != nullptr) delete hcalTriggerGeometry_;
    hcalTriggerGeometry_ = nullptr;
  }

  /**
   * Provides access to the HcalGeometry or HcalQuadGeometry
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
      hcalTriggerGeometry_ = new HcalQuadGeometry(hcalgeom);
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
  HcalQuadGeometry* hcalTriggerGeometry_;
};

}  // namespace hcal
DECLARE_CONDITIONS_PROVIDER_NS(hcal, HcalQuadGeometryProvider);
