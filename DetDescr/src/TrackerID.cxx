#include "DetDescr/TrackerID.h"
#include "DetDescr/DetectorIDInterpreter.h"

namespace ldmx {

  void TrackerID::createInterpreters() {
    IDField::IDFieldList fields;
    fields.push_back(new IDField("subdetector",0,SUBDETECTORID_SHIFT,31));
    fields.push_back(new IDField("layer",1,LAYER_SHIFT,LAYER_SHIFT+IDField::countOnes(LAYER_MASK)-1));
    fields.push_back(new IDField("module",2,MODULE_SHIFT,MODULE_SHIFT+IDField::countOnes(MODULE_MASK)-1));

    DetectorIDInterpreter::registerInterpreter(SD_TRACKER_TAGGER,fields);
    DetectorIDInterpreter::registerInterpreter(SD_TRACKER_RECOIL,fields);
  }

}
