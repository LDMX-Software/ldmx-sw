#include "Hcal/HcalTriggerGeometry.h"
#include <iostream>
#include <sstream>
#include "DetDescr/HcalGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace hcal {

HcalTriggerGeometry::HcalTriggerGeometry(const ldmx::HcalGeometry* hcalGeom)
  : ConditionsObject(CONDITIONS_OBJECT_NAME),
    hcalGeometry_{hcalGeom} {
  
}

std::vector<ldmx::HcalDigiID> HcalTriggerGeometry::contentsOfQuad(
    ldmx::HcalTriggerID triggerCell) const {
  std::vector<ldmx::HcalDigiID> retval;
  for(int iStrip=0;iStrip<4;iStrip++){
    int strip = iStrip+4*triggerCell.superstrip();
    if (strip >= hcalGeometry_->getNumStrips(triggerCell.section())) break;
    retval.push_back( ldmx::HcalDigiID(triggerCell.section(), triggerCell.layer(),
                                       strip, triggerCell.end()) );
  }
  return retval;
}

  std::vector<ldmx::HcalDigiID> HcalTriggerGeometry::contentsOfSTQ(
    ldmx::HcalTriggerID triggerCell) const {
  std::vector<ldmx::HcalDigiID> retval;
  for(int iStrip=0;iStrip<8;iStrip++){
    for(int iLayer=0;iLayer<2;iLayer++){
      int strip = iStrip+8*triggerCell.superstrip();
      if (strip >= hcalGeometry_->getNumStrips(triggerCell.section())) continue;
      int tlayer = triggerCell.layer();
      int player = 2*(tlayer+iStrip) - tlayer%2;
      if (player >= hcalGeometry_->getNumLayers(triggerCell.section())) continue;
      retval.push_back(ldmx::HcalDigiID(triggerCell.section(), player,
                                        strip, triggerCell.end()));
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
