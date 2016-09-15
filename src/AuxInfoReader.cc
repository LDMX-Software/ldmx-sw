#include "SimApplication/AuxInfoReader.h"

// LDMX
#include "SimApplication/TrackerSD.h"

AuxInfoReader::AuxInfoReader(G4GDMLParser* theParser) :
    parser(theParser) {
}

void AuxInfoReader::readGlobalAuxInfo() {

    std::cout << "Reading global aux info from GDML parser" << std::endl;

    const G4GDMLAuxListType* auxInfoList = parser->GetAuxList();
    for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
            iaux != auxInfoList->end(); iaux++ ) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;
        G4String auxUnit = iaux->unit;

        std::cout << "auxType: " << auxType << ", auxVal: " << auxVal << ", auxUnit: " << auxUnit << std::endl;

        if (auxType == "SensitiveDetector") {
            createSensitiveDetector(auxVal, iaux->auxList);
        }
    }
    return;
}

void AuxInfoReader::createSensitiveDetector(G4String sdType, const G4GDMLAuxListType* auxInfoList) {

    std::cout << "creating SensitiveDetector with type " << sdType << std::endl;

    G4String sdName("");
    G4String hcName("");
    for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
                iaux != auxInfoList->end(); iaux++ ) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;
        G4String auxUnit = iaux->unit;

        std::cout << "auxType: " << auxType << ", auxVal: " << auxVal << ", auxUnit: " << auxUnit << std::endl;

        if (auxType == "Name") {
            std::cout << "setting sdName = " << auxVal << std::endl;
            sdName = auxVal;
        } else if (auxType == "HitsCollection") {
            std::cout << "setting hcName = " << auxVal << std::endl;
            hcName = auxVal;
        }
    }

    if (sdName == "") {
        G4Exception("", "", FatalException, "The SensitiveDetector is missing the Name setting.");
    }

    if (sdName == "") {
        G4Exception("", "", FatalException, "The SensitiveDetector is missing the HitsCollection setting.");
    }

    if (sdType == "TrackerSD") {
        std::cout << "Creating new TrackerSD " << sdName << " with hits collection " << hcName << std::endl;
        new TrackerSD(sdName, hcName);
    } else {
        std::cerr << "Bad SensitiveDetector type: " << sdType << std::endl;
        G4Exception("", "", FatalException, "Unknown SensitiveDetector type in aux info.");
    }
}
