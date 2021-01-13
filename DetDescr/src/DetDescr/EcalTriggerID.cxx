#include "DetDescr/EcalTriggerID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::EcalTriggerID& id) {
  s << "EcalTrig(" << id.layer() << ',' << id.module() << ','
    << id.triggercell() << ')';
  return s;
}

namespace ldmx {

void EcalTriggerID::createInterpreters() {
  IDField::IDFieldList fields;
  fields.push_back(new IDField("subdetector", 0, SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("layer", 1, LAYER_SHIFT,
                  LAYER_SHIFT + IDField::countOnes(LAYER_MASK) - 1));
  fields.push_back(
      new IDField("module", 2, MODULE_SHIFT,
                  MODULE_SHIFT + IDField::countOnes(MODULE_MASK) - 1));
  fields.push_back(new IDField("cell", 3, CELL_SHIFT,
                               CELL_SHIFT + IDField::countOnes(CELL_MASK) - 1));

  DetectorIDInterpreter::registerInterpreter(
      SD_ECAL,
      EcalAbstractID::CELL_TYPE_MASK << EcalAbstractID::CELL_TYPE_SHIFT,
      EcalAbstractID::TriggerCell << EcalAbstractID::CELL_TYPE_SHIFT, fields);
}

}  // namespace ldmx
