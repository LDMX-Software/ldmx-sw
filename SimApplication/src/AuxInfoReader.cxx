#include "SimApplication/AuxInfoReader.h"

// LDMX
#include "SimApplication/TrackerSD.h"
#include "SimApplication/CalorimeterSD.h"
#include "SimApplication/MagneticFieldStore.h"
#include "SimApplication/UserRegionInformation.h"
#include "SimApplication/VisAttributesStore.h"
#include "DetDescr/DetectorIDStore.h"

// Geant4
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4FieldManager.hh"
#include "G4UniformMagField.hh"
#include "G4GDMLEvaluator.hh"
#include "G4SystemOfUnits.hh"
#include "G4RegionStore.hh"
#include "G4Region.hh"

// STL
#include <string>
#include <stdlib.h>

using detdescr::DetectorID;
using detdescr::IDField;
using detdescr::DetectorIDStore;

namespace sim {

AuxInfoReader::AuxInfoReader(G4GDMLParser* theParser) :
    parser(theParser), eval(new G4GDMLEvaluator) {
}

AuxInfoReader::~AuxInfoReader() {
    delete eval;
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
        } else if (auxType == "DetectorID") {
            createDetectorID(auxVal, iaux->auxList);
        } else if (auxType == "MagneticField") {
            createMagneticField(auxVal, iaux->auxList);
        } else if (auxType == "Region") {
            createRegion(auxVal, iaux->auxList);
        } else if (auxType == "VisAttributes") {
            createVisAttributes(auxVal, iaux->auxList);
        }
    }
    return;
}

void AuxInfoReader::createSensitiveDetector(G4String theSensDetName, const G4GDMLAuxListType* auxInfoList) {

    std::cout << "Creating SensitiveDetector " << theSensDetName << std::endl;

    G4String sdType("");
    G4String hcName("");
    G4String idName("");
    int subdetID = -1;
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
            subdetID = atoi(auxVal.c_str());
        } else if (auxType == "DetectorID") {
            idName = auxVal;
        }
    }

    if (sdType == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the SensDetType.");
    }

    if (hcName == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the HitsCollection.");
    }

    if (subdetID <= 0 ) {
        std::cerr << "Bad SubdetID: " << subdetID << std::endl;
        G4Exception("", "", FatalException, "The SubdetID is missing or has an invalid value.");
    }

    if (idName == "") {
        G4Exception("", "", FatalException, "The SensDet is missing the DetectorID.");
    }

    DetectorID* detID = DetectorIDStore::getInstance()->getID(idName);
    if (detID == NULL) {
        std::cout << "The Detector ID" << idName << " does not exist.  Is it defined before the SensDet in userinfo?" << std::endl;
        G4Exception("", "", FatalException, "The referenced Detector ID was not found.");
    }

    G4VSensitiveDetector* sd = 0;
    if (sdType == "TrackerSD") {
        sd = new TrackerSD(theSensDetName, hcName, subdetID, detID);
    } else if (sdType == "CalorimeterSD") {
        sd = new CalorimeterSD(theSensDetName, hcName, subdetID, detID);
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

                G4LogicalVolume* lv = (*lvciter);

                if (auxType == "SensDet") {
                    G4String sdName = auxVal;
                    G4VSensitiveDetector* sd = G4SDManager::GetSDMpointer()->FindSensitiveDetector(sdName);
                    if (sd != NULL) {
                        lv->SetSensitiveDetector(sd);
                        std::cout << "Assigned SD " << sd->GetName() << " to " << lv->GetName() << std::endl;
                    } else {
                        std::cout << "Unknown SensDet in volume's auxiliary info: " << sdName << std::endl;
                        G4Exception("", "", FatalException, "The SensDet was not found.  Is it defined in userinfo?");
                    }
                } else if (auxType == "MagneticField") {
                    G4String magFieldName = auxVal;
                    G4MagneticField* magField = MagneticFieldStore::getInstance()->getMagneticField(magFieldName);
                    if (magField != NULL) {
                        G4FieldManager* mgr = new G4FieldManager(magField);
                        lv->SetFieldManager(mgr, true /* FIXME: hard-coded to force field manager to daughters */);
                        std::cout << "Assigned magnetic field " << magFieldName << " to volume " << lv->GetName() << std::endl;
                    } else {
                        std::cout << "Unknown MagneticField ref in volume's auxiliary info: " << magFieldName << std::endl;
                        G4Exception("", "", FatalException, "The MagneticField was not found.  Is it defined in userinfo?");
                    }
                } else if (auxType == "Region") {
                    G4String regionName = auxVal;
                    G4Region* region = G4RegionStore::GetInstance()->GetRegion(regionName);
                    if (region != NULL) {
                        region->AddRootLogicalVolume(lv);
                        std::cout << "Added volume " << lv->GetName() << " to region " << regionName << std::endl;
                    } else {
                        std::cerr << "Referenced region " << regionName << " was not found!" << std::endl;
                        G4Exception("", "", FatalException, "The region was not found.  Is it defined in userinfo?");
                    }
                } else if (auxType == "VisAttributes") {
                    G4String visName = auxVal;
                    G4VisAttributes* visAttributes = VisAttributesStore::getInstance()->getVisAttributes(visName);
                    if (visAttributes != NULL) {
                        lv->SetVisAttributes(visAttributes);
                        std::cout << "Assigned VisAttributes " << visName << " to volume " << lv->GetName() << std::endl;
                    } else {
                        std::cerr << "Referenced VisAttributes " << visName << " was not found!" << std::endl;
                        G4Exception("", "", FatalException, "The VisAttributes was not found.  Is it defined in userinfo?");
                    }
                }
            }
        }
    }
}

void AuxInfoReader::createDetectorID(G4String idName, const G4GDMLAuxListType* auxInfoList) {

    std::cout << "Creating DetectorID " << idName << std::endl;
    IDField::IDFieldList* fieldList = new IDField::IDFieldList();

    // iterate fields
    int fieldIndex = 0;
    for(std::vector<G4GDMLAuxStructType>::const_iterator fieldIt = auxInfoList->begin();
            fieldIt != auxInfoList->end(); fieldIt++ ) {

        std::vector<G4GDMLAuxStructType>* fieldsAux = fieldIt->auxList;

        G4String auxType = fieldIt->type;
        G4String auxVal = fieldIt->value;

        if (auxType == "IDField") {

            std::string fieldName = auxVal;

            std::cout << "Creating IDField " << fieldName << std::endl;

            int startBit, endBit = -1;

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
            }

            if (startBit == -1) {
                G4Exception("", "", FatalException, "The DetectorID is missing the StartBit.");
            }

            if (endBit == -1) {
                G4Exception("", "", FatalException, "The DetectorID is missing the EndBit.");
            }

            fieldList->push_back(new IDField(fieldName, fieldIndex, startBit, endBit));

            std::cout << "Added IDField " << fieldName << " with StartBit = " << startBit << ", EndBit = "
                    << endBit << ", Index = " << fieldIndex << std::endl;

            // Increment field index which is assigned automatically based on element ordering.
            fieldIndex++;
        }
    }

    DetectorID* id = new DetectorID(fieldList);
    DetectorIDStore::getInstance()->addID(idName, id);
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

void AuxInfoReader::createRegion(G4String name, const G4GDMLAuxListType* auxInfoList) {

    bool storeTrajectories = true;
    for(std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
                iaux != auxInfoList->end(); iaux++ ) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;
        G4String auxUnit = iaux->unit;

        if (auxType == "StoreTrajectories") {
            if (auxVal == "false") {
                storeTrajectories = false;
            } else if (auxVal == "true") {
                storeTrajectories = true;
            }
        }
    }

    G4VUserRegionInformation* regionInfo = new UserRegionInformation(storeTrajectories);
    G4Region* region = new G4Region(name);
    region->SetUserInformation(regionInfo);

    std::cout << "Created new detector region " << region->GetName() << std::endl << std::endl;
}

void AuxInfoReader::createVisAttributes(G4String name, const G4GDMLAuxListType* auxInfoList) {

    G4double rgba[4];
    rgba[0] = rgba[1] = rgba[2] = 0;
    rgba[3] = 10.;
    G4bool visible = true;
    G4bool dauInvisible = true;
    G4bool forceWireframe = false;
    G4bool forceSolid = false;
    G4double lineWidth = 1.0;
    G4VisAttributes::LineStyle lineStyle = G4VisAttributes::unbroken;

    for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
            iaux != auxInfoList->end(); iaux++) {

        G4String auxType = iaux->type;
        G4String auxVal = iaux->value;
        G4String auxUnit = iaux->unit;

        if (auxType == "R") {
            rgba[0] = atof(auxVal.c_str());
        } else if (auxType == "G") {
            rgba[1] = atof(auxVal.c_str());
        } else if (auxType == "B") {
            rgba[2] = atof(auxVal.c_str());
        } else if (auxType == "A") {
            rgba[3] = atof(auxVal.c_str());
        } else if (auxType == "Style") {
            if (auxVal == "wireframe") {
                forceWireframe = true;
            } else if (auxVal == "solid") {
                forceSolid = true;
            }
        } else if (auxType == "DaughtersInvisible") {
            if (auxVal == "true") {
                dauInvisible = true;
            } else if (auxVal == "false") {
                dauInvisible = false;
            }
        } else if (auxType == "Visible") {
            if (auxVal == "true") {
                visible = true;
            } else if (auxVal == "false") {
                visible = false;
            }
        } else if (auxType == "LineStyle") {
            if (auxVal == "unbroken") {
                lineStyle = G4VisAttributes::unbroken;
            } else if (auxVal == "dashed") {
                lineStyle = G4VisAttributes::dashed;
            } else if (auxVal == "dotted") {
                lineStyle = G4VisAttributes::dotted;
            }
        } else if (auxType == "LineWidth") {
            lineWidth = atof(auxVal.c_str());
        }
    }

    G4VisAttributes* visAttributes = new G4VisAttributes();
    visAttributes->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    visAttributes->SetVisibility(visible);
    visAttributes->SetDaughtersInvisible(dauInvisible);
    visAttributes->SetForceWireframe(forceWireframe);
    visAttributes->SetForceSolid(forceSolid);
    visAttributes->SetLineWidth(lineWidth);
    visAttributes->SetLineStyle(lineStyle);
    VisAttributesStore::getInstance()->addVisAttributes(name, visAttributes);

    std::cout << "Created VisAttributes " << name << std::endl << (*visAttributes)
            << std::endl << std::endl;
}

}
