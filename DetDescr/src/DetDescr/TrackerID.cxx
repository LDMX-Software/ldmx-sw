#include "DetDescr/TrackerID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::TrackerID& id) {
  if (id.null())
    s << "NULL";
  else if (id.subdet() == ldmx::SD_TRACKER_RECOIL)
    s << "Recoil(";
  else if (id.subdet() == ldmx::SD_TRACKER_TAGGER)
    s << "Tagger(";
  else
    s << "UnknownTk(";
  s << id.layer() << ',' << id.module() << ')';
  return s;
}

namespace ldmx {

void TrackerID::createInterpreters() {
  IDField::IDFieldList fields;
  fields.push_back(new IDField("subdetector", 0, SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("layer", 1, LAYER_SHIFT,
                  LAYER_SHIFT + IDField::countOnes(LAYER_MASK) - 1));
  fields.push_back(
      new IDField("module", 2, MODULE_SHIFT,
                  MODULE_SHIFT + IDField::countOnes(MODULE_MASK) - 1));

  DetectorIDInterpreter::registerInterpreter(SD_TRACKER_TAGGER, fields);
  DetectorIDInterpreter::registerInterpreter(SD_TRACKER_RECOIL, fields);
}

}  // namespace ldmx
