#include "DetDescr/TrigScintID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::TrigScintID& id) {
  s << "TrigScint(" << id.module() << ',' << id.bar() << ')';
  return s;
}

namespace ldmx {
void TrigScintID::createInterpreters() {
  IDField::IDFieldList fields;
  fields.push_back(new IDField("subdetector", 0, SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("module", 1, MODULE_SHIFT,
                  MODULE_SHIFT + IDField::countOnes(MODULE_MASK) - 1));
  fields.push_back(new IDField("bar", 2, BAR_SHIFT,
                               BAR_SHIFT + IDField::countOnes(BAR_MASK) - 1));

  DetectorIDInterpreter::registerInterpreter(SD_TRIGGER_SCINT, fields);
}
}  // namespace ldmx
