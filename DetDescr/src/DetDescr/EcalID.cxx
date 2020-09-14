#include "DetDescr/EcalID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::EcalID& id) {
  s << "Ecal(" << id.layer() << ',' << id.module() << ',' << id.cell() << ')';
  return s;
}

namespace ldmx {

  void EcalID::createInterpreters() {
    IDField::IDFieldList fields;
    fields.push_back(new IDField("subdetector",0,SUBDETECTORID_SHIFT,31));
    fields.push_back(new IDField("layer",1,LAYER_SHIFT,LAYER_SHIFT+IDField::countOnes(LAYER_MASK)-1));
    fields.push_back(new IDField("module",2,MODULE_SHIFT,MODULE_SHIFT+IDField::countOnes(MODULE_MASK)-1));
    fields.push_back(new IDField("cell",3,CELL_SHIFT,CELL_SHIFT+IDField::countOnes(CELL_MASK)-1));

    DetectorIDInterpreter::registerInterpreter(SD_ECAL,fields);

  }

}
