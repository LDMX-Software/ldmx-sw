#include "SimApplication/HcalSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

namespace ldmx {

HcalSD::HcalSD(G4String name, G4String theCollectionName, DetectorID* detID) :
		CalorimeterSD(name,theCollectionName, detID) {;}

HcalSD::~HcalSD() {;}

}
