#include "SimApplication/AuxInfoReader.h"

// LDMX
#include "SimApplication/TrackerSD.h"

// Geant4
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"

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

        //std::cout << "auxType: " << auxType << ", auxVal: " << auxVal << ", auxUnit: " << auxUnit << std::endl;

        if (auxType == "SensDet") {
            createSensitiveDetector(auxVal, iaux->auxList);
        }
    }
    return;
}

void AuxInfoReader::createSensitiveDetector(G4String theSensDetName, const G4GDMLAuxListType* auxInfoList) {

    std::cout << "creating SensitiveDetector " << theSensDetName << std::endl;

    G4String sdType("");
    G4String hcName("");
    int verbose = 0;
    for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
                iaux != auxInfoList->end(); iaux++ ) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;
        G4String auxUnit = iaux->unit;

        std::cout << "auxType: " << auxType << ", auxVal: " << auxVal << ", auxUnit: " << auxUnit << std::endl;

        if (auxType == "SensDetType") {
            std::cout << "setting SensDetType = " << auxVal << std::endl;
            sdType = auxVal;
        } else if (auxType == "HitsCollection") {
            std::cout << "setting HitsCollection = " << auxVal << std::endl;
            hcName = auxVal;
        } else if (auxType == "Verbose") {
            std::cout << "setting Verbose = " << auxVal << std::endl;
            verbose = atoi(auxVal.c_str());
        }
    }

    if (sdType == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the SensDetType.");
    }

    if (hcName == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the HitsCollection.");
    }

    G4VSensitiveDetector* sd = 0;
    if (sdType == "TrackerSD") {
        std::cout << "Creating new TrackerSD " << theSensDetName << " with hits collection " << hcName
                << " and verbose set to " << verbose << std::endl;
        sd = new TrackerSD(theSensDetName, hcName);
    } else {
        std::cerr << "Bad SensitiveDetector type: " << sdType << std::endl;
        G4Exception("", "", FatalException, "Unknown SensitiveDetector type in aux info.");
    }

    sd->SetVerboseLevel(verbose);
}

void AuxInfoReader::assignSensDetsToVols() {
    const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
    std::vector<G4LogicalVolume*>::const_iterator lvciter;
    for(lvciter = lvs->begin(); lvciter != lvs->end(); lvciter++) {
        G4GDMLAuxListType auxInfo = parser->GetVolumeAuxiliaryInformation(*lvciter);
        if (auxInfo.size() > 0) {
            G4cout << "Auxiliary Information is found for Logical Volume :  "
                    << (*lvciter)->GetName() << G4endl;

            for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfo.begin();
                    iaux != auxInfo.end(); iaux++ ) {

                G4String auxType = iaux->type;
                G4String auxVal = iaux->value;
                G4String auxUnit = iaux->unit;

                std::cout << "  auxType: " << auxType << ", auxVal: " << auxVal << ", auxUnit: " << auxUnit << std::endl;

                if (auxType == "SensDet") {
                    G4String sdName = auxVal;
                    G4VSensitiveDetector* sd = G4SDManager::GetSDMpointer()->FindSensitiveDetector(sdName);
                    if (sd != 0) {
                        std::cout << "assigning SD " << sd->GetName() << " to LV " << (*lvciter)->GetName() << std::endl;
                        (*lvciter)->SetSensitiveDetector(sd);
                    } else {
                        std::cout << "Unknown SensDet: " << sdName << std::endl;
                        G4Exception("", "", FatalException, "The SensDet was not found.  Is it defined in userinfo?");
                    }
                }
            }
        }
    }
}
