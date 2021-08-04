#include "DetDescr/HcalTriggerID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::HcalTriggerID& id) {
  s << "HcalTrig(" << id.section() << ',' << id.layer() << ','
    << id.superstrip() << ')';
  return s;
}

namespace ldmx {

void HcalTriggerID::createInterpreters() {
  IDField::IDFieldList fields;
  fields.push_back(new IDField("subdetector", 0, SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("section", 1, SECTION_SHIFT,
                  SECTION_SHIFT + IDField::countOnes(SECTION_MASK) - 1));
  fields.push_back(
      new IDField("layer", 2, LAYER_SHIFT,
                  LAYER_SHIFT + IDField::countOnes(LAYER_MASK) - 1));
  fields.push_back(
      new IDField("superstrip", 3, SUPERSTRIP_SHIFT,
                  SUPERSTRIP_SHIFT + IDField::countOnes(SUPERSTRIP_MASK) - 1));

  DetectorIDInterpreter::registerInterpreter(
      SD_HCAL, HcalAbstractID::BAR_TYPE_MASK << HcalAbstractID::BAR_TYPE_SHIFT,
      HcalAbstractID::Trigger << HcalAbstractID::BAR_TYPE_SHIFT, fields);
}

}  // namespace ldmx
