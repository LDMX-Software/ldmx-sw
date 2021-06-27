#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalAbstractID.h"
#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/HcalAbstractID.h"
#include "DetDescr/SimSpecialID.h"
#include "DetDescr/TrackerID.h"
#include "DetDescr/TrigScintID.h"
#include "DetDescr/SimSpecialID.h"
#include <iomanip>

std::ostream& operator<<(std::ostream& s, const ldmx::DetectorID& id) {
  switch (id.subdet()) {
    case(ldmx::SD_TRACKER_TAGGER):
    case(ldmx::SD_TRACKER_RECOIL): return s << ldmx::TrackerID(id);
    case(ldmx::SD_TRIGGER_SCINT): return s << ldmx::TrigScintID(id);
    case(ldmx::SD_ECAL): return s << ldmx::EcalAbstractID(id);
    case(ldmx::SD_HCAL): return s << ldmx::HcalAbstractID(id);
    case(ldmx::SD_SIM_SPECIAL): return s << ldmx::SimSpecialID(id);
    case(ldmx::EID_ECAL): return s << ldmx::EcalElectronicsID(id);
    default:
      return s << "DetectorID("<<id.subdet()<<":0x"<<std::setfill('0') << std::setw(2) << std::right << std::hex << (id.raw() & ldmx::DetectorID::
                                                                                                                     SUBDETECTOR_PAYLOAD_MASK);
  }
}

