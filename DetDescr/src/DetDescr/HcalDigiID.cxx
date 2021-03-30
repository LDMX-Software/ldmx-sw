#include "DetDescr/HcalDigiID.h"

#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::HcalDigiID& id) {
  s << "Hcal(" << id.section() << ',' << id.layer() << ',' << id.strip() << ','
    << id.end() << ')';
  return s;
}

// Section: 0 Back Hcal, > 0 Side Hcal
// End: 0 Top/Left (positive y/x), 1 Bottom/Right (negative y/x)
namespace ldmx {

void HcalDigiID::createInterpreters() {
  IDField::IDFieldList fields;
  fields.push_back(new IDField("subdetector", 0, SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("section", 1, SECTION_SHIFT,
                  SECTION_SHIFT + IDField::countOnes(SECTION_MASK) - 1));
  fields.push_back(
      new IDField("layer", 2, LAYER_SHIFT,
                  LAYER_SHIFT + IDField::countOnes(LAYER_MASK) - 1));
  fields.push_back(
      new IDField("strip", 3, STRIP_SHIFT,
                  STRIP_SHIFT + IDField::countOnes(STRIP_MASK) - 1));
  fields.push_back(new IDField("end", 4, END_SHIFT,
                               END_SHIFT + IDField::countOnes(END_MASK) - 1));

  DetectorIDInterpreter::registerInterpreter(
      SD_HCAL, HcalAbstractID::BAR_TYPE_MASK << HcalAbstractID::BAR_TYPE_SHIFT,
      HcalAbstractID::Digi << HcalAbstractID::BAR_TYPE_SHIFT, fields);
}

}  // namespace ldmx
