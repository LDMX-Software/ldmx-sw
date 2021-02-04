#include "DetDescr/SimSpecialID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::SimSpecialID& id) {
  if (id.getSubtype() == ldmx::SimSpecialID::SCORING_PLANE)
    s << "SimSpecial(ScoringPlane " << id.plane() << ')';
  else
    s << "SimSpecial(Type " << id.getSubtype() << ',' << id.subtypePayload()
      << ')';
  return s;
}

namespace ldmx {

void SimSpecialID::createInterpreters() {
  IDField::IDFieldList fields;
  fields.push_back(new IDField("subdetector", 0, SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("subtype", 1, SUBTYPE_SHIFT,
                  SUBTYPE_SHIFT + IDField::countOnes(SUBTYPE_MASK) - 1));
  fields.push_back(
      new IDField("payload", 2, 0, IDField::countOnes(SUBTYPE_DATA_MASK) - 1));

  DetectorIDInterpreter::registerInterpreter(SD_SIM_SPECIAL, fields);
}

}  // namespace ldmx
