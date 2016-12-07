#ifndef SIMAPPLICATION_HCALSD_H_
#define SIMAPPLICATION_HCALSD_H_

// LDMX
#include "SimApplication/CalorimeterSD.h"

using detdescr::DetectorID;
using event::Event;

namespace sim {

class HcalSD : public CalorimeterSD {

    public:

		HcalSD(G4String name,
				G4String theCollectionName,
				int subdet,
				DetectorID* detID);

		virtual ~HcalSD();


};

}

#endif
