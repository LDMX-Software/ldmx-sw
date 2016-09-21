#include "SimApplication/AuxInfoReader.h"

// LDMX
#include "SimApplication/TrackerSD.h"
#include "SimApplication/CalorimeterSD.h"
#include "DetDescr/DetectorIdStore.h"

// Geant4
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"

AuxInfoReader::AuxInfoReader(G4GDMLParser* theParser) :
    parser(theParser) {
}

void AuxInfoReader::readGlobalAuxInfo() {

    const G4GDMLAuxListType* auxInfoList = parser->GetAuxList();
    for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
            iaux != auxInfoList->end(); iaux++ ) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;
        G4String auxUnit = iaux->unit;

        if (auxType == "SensDet") {
            createSensitiveDetector(auxVal, iaux->auxList);
        } else if (auxType == "DetectorId") {
            createDetectorId(auxVal, iaux->auxList);
        }
    }
    return;
}

void AuxInfoReader::createSensitiveDetector(G4String theSensDetName, const G4GDMLAuxListType* auxInfoList) {

    std::cout << "Creating SensitiveDetector " << theSensDetName << std::endl;

    G4String sdType("");
    G4String hcName("");
    G4String idName("");
    int subdetId = -1;
    int verbose = 0;
    for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
                iaux != auxInfoList->end(); iaux++ ) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;
        G4String auxUnit = iaux->unit;

        std::cout << "auxType: " << auxType << ", auxVal: " << auxVal << ", auxUnit: " << auxUnit << std::endl;

        if (auxType == "SensDetType") {
            sdType = auxVal;
        } else if (auxType == "HitsCollection") {
            hcName = auxVal;
        } else if (auxType == "Verbose") {
            verbose = atoi(auxVal.c_str());
        } else if (auxType == "SubdetID") {
            subdetId = atoi(auxVal.c_str());
        } else if (auxType == "DetectorId") {
            idName = auxVal;
        }
    }

    if (sdType == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the SensDetType.");
    }

    if (hcName == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the HitsCollection.");
    }

    if (subdetId <= 0 ) {
        std::cerr << "Bad SubdetID: " << subdetId << std::endl;
        G4Exception("", "", FatalException, "The SubdetID is missing or has an invalid value.");
    }

    if (idName == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the DetectorId.");
    }

    DetectorId* detId = DetectorIdStore::getInstance()->getId(idName);
    if (detId == NULL) {
        std::cout << "The Detector ID" << idName << " does not exist.  Is it defined before the SensDet in userinfo?" << std::endl;
        G4Exception("", "", FatalException, "The referenced Detector ID was not found.");
    }

    G4VSensitiveDetector* sd = 0;
    if (sdType == "TrackerSD") {
        sd = new TrackerSD(theSensDetName, hcName, subdetId, detId);
    } else if (sdType == "CalorimeterSD") {
        sd = new CalorimeterSD(theSensDetName, hcName, subdetId, detId);
    } else {
        std::cerr << "Unknown SensitiveDetector type: " << sdType << std::endl;
        G4Exception("", "", FatalException, "Unknown SensitiveDetector type in aux info.");
    }

    sd->SetVerboseLevel(verbose);

    std::cout << "Created " << sdType << " " << theSensDetName
            << " with hits collection " << hcName
            << " and verbose level " << verbose << std::endl << std::endl;
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

                if (auxType == "SensDet") {
                    G4String sdName = auxVal;
                    G4VSensitiveDetector* sd = G4SDManager::GetSDMpointer()->FindSensitiveDetector(sdName);
                    if (sd != 0) {
                        std::cout << "Assigning SD " << sd->GetName() << " to " << (*lvciter)->GetName() << std::endl;
                        (*lvciter)->SetSensitiveDetector(sd);
                    } else {
                        std::cout << "Unknown SensDet ref in volume's auxiliary info: " << sdName << std::endl;
                        G4Exception("", "", FatalException, "The SensDet was not found.  Is it defined in userinfo?");
                    }
                }
            }
        }
    }
}

void AuxInfoReader::createDetectorId(G4String idName, const G4GDMLAuxListType* auxInfoList) {

    std::cout << "creating DetectorId " << idName << std::endl;
    IdField::IdFieldList* fieldList = new IdField::IdFieldList();

    // iterate fields
    for(std::vector<G4GDMLAuxStructType>::const_iterator fieldIt = auxInfoList->begin();
            fieldIt != auxInfoList->end(); fieldIt++ ) {

        std::vector<G4GDMLAuxStructType>* fieldsAux = fieldIt->auxList;

        G4String auxType = fieldIt->type;
        G4String auxVal = fieldIt->value;

        if (auxType == "IdField") {

            std::string fieldName = auxVal;

            std::cout << "Creating IdField " << fieldName << std::endl;

            int startBit, endBit = -1;
            int index = 0;

            // iterate field aux values
            for(std::vector<G4GDMLAuxStructType>::const_iterator fieldValsIt = fieldsAux->begin();
                    fieldValsIt != fieldsAux->end(); fieldValsIt++ ) {

                G4String fieldValAuxType = fieldValsIt->type;
                G4String fieldValAuxValue = fieldValsIt->value;

                if (fieldValAuxType == "StartBit") {
                    startBit = atoi(fieldValAuxValue.c_str());
                } else if (fieldValAuxType == "EndBit") {
                    endBit = atoi(fieldValAuxValue.c_str());
                }

                // Increment field index which is assigned automatically based on element ordering.
                index++;
            }

            if (startBit == -1) {
                G4Exception("", "", FatalException, "The DetectorId is missing the StartBit.");
            }

            if (endBit == -1) {
                G4Exception("", "", FatalException, "The DetectorId is missing the EndBit.");
            }

            fieldList->push_back(new IdField(fieldName, index, startBit, endBit));

            std::cout << "Added IdField " << fieldName << " with StartBit = " << startBit << ", EndBit = "
                    << endBit << ", Index = " << index << std::endl;
        }
    }

    DetectorId* id = new DetectorId(fieldList);
    DetectorIdStore::getInstance()->addId(idName, id);
    std::cout << "Created detector ID " << idName << std::endl << std::endl;
}
