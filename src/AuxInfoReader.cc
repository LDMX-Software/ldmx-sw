#include "SimApplication/AuxInfoReader.h"

// LDMX
#include "SimApplication/TrackerSD.h"
#include "SimApplication/CalorimeterSD.h"
#include "SimApplication/MagneticFieldStore.h"
#include "DetDescr/DetectorIdStore.h"

// Geant4
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4FieldManager.hh"
#include "G4UniformMagField.hh"
#include "G4GDMLEvaluator.hh"
#include "G4SystemOfUnits.hh"

// STL
#include <string>

AuxInfoReader::AuxInfoReader(G4GDMLParser* parser) :
    parser(parser), eval(new G4GDMLEvaluator) {
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
        } else if (auxType == "MagneticField") {
            createMagneticField(auxVal, iaux->auxList);
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
        } else if (auxType == "SubdetId") {
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
        std::cerr << "Bad SubdetId: " << subdetId << std::endl;
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

void AuxInfoReader::assignAuxInfoToVolumes() {
    const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
    std::vector<G4LogicalVolume*>::const_iterator lvciter;
    for(lvciter = lvs->begin(); lvciter != lvs->end(); lvciter++) {
        G4GDMLAuxListType auxInfo = parser->GetVolumeAuxiliaryInformation(*lvciter);
        if (auxInfo.size() > 0) {

            for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfo.begin();
                    iaux != auxInfo.end(); iaux++ ) {

                G4String auxType = iaux->type;
                G4String auxVal = iaux->value;
                G4String auxUnit = iaux->unit;

                G4LogicalVolume* logVol = (*lvciter);

                if (auxType == "SensDet") {
                    G4String sdName = auxVal;
                    G4VSensitiveDetector* sd = G4SDManager::GetSDMpointer()->FindSensitiveDetector(sdName);
                    if (sd != NULL) {
                        logVol->SetSensitiveDetector(sd);
                        std::cout << "Assiged SD " << sd->GetName() << " to " << logVol->GetName() << std::endl;
                    } else {
                        std::cout << "Unknown SensDet in volume's auxiliary info: " << sdName << std::endl;
                        G4Exception("", "", FatalException, "The SensDet was not found.  Is it defined in userinfo?");
                    }
                } else if (auxType == "MagneticField") {
                    G4String magFieldName = auxVal;
                    G4MagneticField* magField = MagneticFieldStore::getInstance()->getMagneticField(magFieldName);
                    if (magField != NULL) {
                        G4FieldManager* mgr = new G4FieldManager(magField);
                        logVol->SetFieldManager(mgr, true /* FIXME: hard-coded to force field manager to daughters */);
                        std::cout << "Assigned magnetic field " << magFieldName << " to volume " << logVol->GetName() << std::endl;
                    } else {
                        std::cout << "Unknown MagneticField ref in volume's auxiliary info: " << magFieldName << std::endl;
                        G4Exception("", "", FatalException, "The MagneticField was not found.  Is it defined in userinfo?");
                    }
                }
            }
        }
    }
}

void AuxInfoReader::createDetectorId(G4String idName, const G4GDMLAuxListType* auxInfoList) {

    std::cout << "Creating DetectorId " << idName << std::endl;
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

void AuxInfoReader::createMagneticField(G4String magFieldName, const G4GDMLAuxListType* auxInfoList) {

    // Find type of the mag field.
    G4String magFieldType("");
    for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
                    iaux != auxInfoList->end(); iaux++ ) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;

        if (auxType == "MagneticFieldType") {
            magFieldType = auxVal;
            break;
        }
    }

    if (magFieldType == "") {
        G4Exception("", "", FatalException, "Missing MagFieldType for magnetic field definition.");
    }

    G4MagneticField* magField = NULL;

    // Create a uniform mag field using the built-in Geant4 type.
    if (magFieldType == "G4UniformMagField") {
        std::string::size_type sz;
        double bx, by, bz;
        bx = by = bz = 0.;
        for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

            G4String auxType = iaux->type;
            G4String auxVal = iaux->value;
            G4String auxUnit = iaux->unit;

            G4String expr = auxVal + "*" + auxUnit;
            if (auxType == "bx") {
                bx = eval->Evaluate(expr);
            } else if (auxType == "by") {
                by = eval->Evaluate(expr);
            } else if (auxType == "bz") {
                bz = eval->Evaluate(expr);
            }
        }
        G4ThreeVector fieldComponents(bx, by, bz);
        magField = new G4UniformMagField(fieldComponents);

        std::cout << "Created G4UniformMagField " << magFieldName << " with field components " << fieldComponents << std::endl << std::endl;

    } else {
        std::cerr << "Unknown MagFieldType in auxiliary info: " << magFieldType << std::endl;
        G4Exception("", "", FatalException, "Unknown MagFieldType in auxiliary info.");
    }

    MagneticFieldStore::getInstance()->addMagneticField(magFieldName, magField);
}
