#include "SimApplication/HcalSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

// LDMX
#include "Event/RootEventWriter.h"

using event::RootEventWriter;

namespace sim {

HcalSD::HcalSD(G4String name, G4String theCollectionName, int subdetID, DetectorID* detID) :
		CalorimeterSD(name,theCollectionName,subdetID,detID){};

HcalSD::~HcalSD()  {}

}
