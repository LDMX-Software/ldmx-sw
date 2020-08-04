#include "SimCore/AuxInfoReader.h"

// LDMX
#include "Framework/Exception.h"
#include "SimCore/TrackerSD.h"
#include "SimCore/CalorimeterSD.h"
#include "SimCore/EcalSD.h"
#include "SimCore/HcalSD.h"
#include "SimCore/TrigScintSD.h" 
#include "SimCore/ScoringPlaneSD.h"
#include "SimCore/MagneticFieldStore.h"
#include "SimCore/MagneticFieldMap3D.h"
#include "SimCore/UserRegionInformation.h"
#include "SimCore/VisAttributesStore.h"

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

using std::string;

namespace ldmx {

    AuxInfoReader::AuxInfoReader(G4GDMLParser* theParser, Parameters ps) :
            parser_(theParser), eval_(new G4GDMLEvaluator), parameters_(ps) {
    }

    AuxInfoReader::~AuxInfoReader() {
        delete eval_;
        delete detectorHeader_;
    }

    void AuxInfoReader::readGlobalAuxInfo() {

        const G4GDMLAuxListType* auxInfoList = parser_->GetAuxList();
        for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

            G4String auxType = iaux->type;
            G4String auxVal = iaux->value;
            G4String auxUnit = iaux->unit;

            if (auxType == "SensDet") {
                createSensitiveDetector(auxVal, iaux->auxList);
            } else if (auxType == "MagneticField") {
                createMagneticField(auxVal, iaux->auxList);
            } else if (auxType == "Region") {
                createRegion(auxVal, iaux->auxList);
            } else if (auxType == "VisAttributes") {
                createVisAttributes(auxVal, iaux->auxList);
            } else if (auxType == "DetectorVersion") {
                createDetectorHeader(auxVal, iaux->auxList);
            }
        }
        return;
    }

    void AuxInfoReader::createSensitiveDetector(G4String theSensDetName, const G4GDMLAuxListType* auxInfoList) {

        //G4cout << "Creating SensitiveDetector " << theSensDetName << G4endl;

        G4String sdType("");
        G4String hcName("");
        G4String idName("");
        int subdetID = -1;
        int layerDepth = -1;
        int verbose = 0;
        for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

            G4String auxType = iaux->type;
            G4String auxVal = iaux->value;
            G4String auxUnit = iaux->unit;

            //G4cout << "auxType: " << auxType << ", auxVal: " << auxVal << ", auxUnit: " << auxUnit << G4endl;

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
            } else if (auxType == "LayerDepth") {
                layerDepth = atoi(auxVal.c_str());
            }
        }

        if (sdType == "") {
            EXCEPTION_RAISE( "MissingInfo" , "The SensDet is missing the SensDetType." );
        }

        if (hcName == "") {
            EXCEPTION_RAISE( "MissingInfo" , "The SensDet is missing the HitsCollection." );
        }

        if (subdetID <= 0) {
            EXCEPTION_RAISE( "BadID" , "The SubdetID '" + std::to_string(subdetID) + "' is missing or invalid." );
        }

        /*
         * Build the Sensitive Detector, and re-assign the detID if applicable
         */
        G4VSensitiveDetector* sd = 0;

        if (sdType == "TrackerSD") {
            sd = new TrackerSD(theSensDetName, hcName, subdetID);
        } else if (sdType == "EcalSD") {
            sd = new EcalSD(theSensDetName, hcName, subdetID);
            dynamic_cast<EcalSD*>(sd)->configure(parameters_);
        } else if (sdType == "HcalSD") {
            sd = new HcalSD(theSensDetName, hcName, subdetID);
        } else if (sdType == "ScoringPlaneSD") { 
            sd = new ScoringPlaneSD(theSensDetName, hcName, subdetID); 
        } else if (sdType == "TrigScintSD") { 
            sd = new TrigScintSD(theSensDetName, hcName, subdetID);
        } else {
            EXCEPTION_RAISE( "DetType" , "Unknown SensitiveDetector type: " + sdType );
        }

        /*
         * Fix  layer depth if the Sensitive Detector is not the Tracker
         */
        if (sdType != "TrackerSD" && layerDepth != -1) {
            ((CalorimeterSD*) sd)->setLayerDepth(layerDepth);
        }
        sd->SetVerboseLevel(verbose);

        //G4cout << "Created " << sdType << " " << theSensDetName << " with hits collection " << hcName << " and verbose level " << verbose << G4endl << G4endl;
    }

    void AuxInfoReader::assignAuxInfoToVolumes() {
        const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
        std::vector<G4LogicalVolume*>::const_iterator lvciter;
        for (lvciter = lvs->begin(); lvciter != lvs->end(); lvciter++) {
            G4GDMLAuxListType auxInfo = parser_->GetVolumeAuxiliaryInformation(*lvciter);
            if (auxInfo.size() > 0) {

                for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfo.begin(); iaux != auxInfo.end(); iaux++) {

                    G4String auxType = iaux->type;
                    G4String auxVal = iaux->value;
                    G4String auxUnit = iaux->unit;

                    G4LogicalVolume* lv = (*lvciter);

                    if (auxType == "SensDet") {
                        G4String sdName = auxVal;
                        G4VSensitiveDetector* sd = G4SDManager::GetSDMpointer()->FindSensitiveDetector(sdName);
                        if (sd != NULL) {
                            lv->SetSensitiveDetector(sd);
                            //G4cout << "Assigned SD " << sd->GetName() << " to " << lv->GetName() << G4endl;
                        } else {
                            EXCEPTION_RAISE( "MissingInfo" , "Unknown SensDet in volume's auxiliary info: " + std::string(sdName.data()) );
                        }
                    } else if (auxType == "MagneticField") {
                        G4String magFieldName = auxVal;
                        G4MagneticField* magField = MagneticFieldStore::getInstance()->getMagneticField(magFieldName);
                        if (magField != NULL) {
                            G4FieldManager* mgr = new G4FieldManager(magField);
                            lv->SetFieldManager(mgr, true /* FIXME: hard-coded to force field manager to daughters */);
                            //G4cout << "Assigned magnetic field " << magFieldName << " to volume " << lv->GetName() << G4endl;
                        } else {
                            EXCEPTION_RAISE( "MissingInfo" , "Unknown MagneticField ref in volume's auxiliary info: " + std::string(magFieldName.data()) );
                        }
                    } else if (auxType == "Region") {
                        G4String regionName = auxVal;
                        G4Region* region = G4RegionStore::GetInstance()->GetRegion(regionName);
                        if (region != NULL) {
                            region->AddRootLogicalVolume(lv);
                            //G4cout << "Added volume " << lv->GetName() << " to region " << regionName << G4endl;
                        } else {
                            EXCEPTION_RAISE( "MissingInfo" , "Reference region '" + std::string(regionName.data()) + "' was not found!" );
                        }
                    } else if (auxType == "VisAttributes") {
                        G4String visName = auxVal;
                        G4VisAttributes* visAttributes = VisAttributesStore::getInstance()->getVisAttributes(visName);
                        if (visAttributes != NULL) {
                            lv->SetVisAttributes(visAttributes);
                            //G4cout << "Assigned VisAttributes " << visName << " to volume " << lv->GetName() << G4endl;
                        } else {
                            EXCEPTION_RAISE( "MissingInfo" , "Referenced VisAttributes '" + std::string(visName.data()) + "' was not found!" );
                        }
                    }
                }
            }
        }
    }

    void AuxInfoReader::createMagneticField(G4String magFieldName, const G4GDMLAuxListType* auxInfoList) {

        // Find type of the mag field.
        G4String magFieldType("");
        for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

            G4String auxType = iaux->type;
            G4String auxVal = iaux->value;

            if (auxType == "MagneticFieldType") {
                magFieldType = auxVal;
                break;
            }
        }

        if (magFieldType == "") {
            EXCEPTION_RAISE( "MissingInfo" , "Missing MagFieldType for magnetic field definition." );
        }

        G4MagneticField* magField = NULL;

        // Create a uniform mag field using the built-in Geant4 type.
        if (magFieldType == "G4UniformMagField") {
            string::size_type sz;
            double bx, by, bz;
            bx = by = bz = 0.;
            for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

                G4String auxType = iaux->type;
                G4String auxVal = iaux->value;
                G4String auxUnit = iaux->unit;

                G4String expr = auxVal + "*" + auxUnit;
                if (auxType == "bx") {
                    bx = eval_->Evaluate(expr);
                } else if (auxType == "by") {
                    by = eval_->Evaluate(expr);
                } else if (auxType == "bz") {
                    bz = eval_->Evaluate(expr);
                }
            }
            G4ThreeVector fieldComponents(bx, by, bz);
            magField = new G4UniformMagField(fieldComponents);

            //G4cout << "Created G4UniformMagField " << magFieldName << " with field components " << fieldComponents << G4endl << G4endl;

            // Create a global 3D field map by reading from a data file.
        } else if (magFieldType == "MagneticFieldMap3D") {

            string fileName;
            double offsetX, offsetY, offsetZ;

            for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

                G4String auxType = iaux->type;
                G4String auxVal = iaux->value;
                G4String auxUnit = iaux->unit;

                G4String expr = auxVal + "*" + auxUnit;

                if (auxType == "File") {
                    fileName = auxVal;
                } else if (auxType == "OffsetX") {
                    offsetX = eval_->Evaluate(expr);
                } else if (auxType == "OffsetY") {
                    offsetY = eval_->Evaluate(expr);
                } else if (auxType == "OffsetZ") {
                    offsetZ = eval_->Evaluate(expr);
                }
            }

            if (fileName.size() == 0) {
                EXCEPTION_RAISE( "MissingInfo" , "File info with field data was not provided." );
            }

            // Create new 3D field map.
            magField = new MagneticFieldMap3D(fileName.c_str(), offsetX, offsetY, offsetZ);

            // Assign field map as global field.
            G4FieldManager* fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
            if (fieldMgr->GetDetectorField() != nullptr) {
                EXCEPTION_RAISE( "MisAssign" , "Global mag field was already assigned." );
            }
            fieldMgr->SetDetectorField(magField);
            fieldMgr->CreateChordFinder(magField);

        } else {
            EXCEPTION_RAISE( "UnknownType" , "Unknown MagFieldType '" + std::string(magFieldType.data()) + "' in auxiliary info." );
        }

        MagneticFieldStore::getInstance()->addMagneticField(magFieldName, magField);
    }

    void AuxInfoReader::createRegion(G4String name, const G4GDMLAuxListType* auxInfoList) {

        bool storeTrajectories = true;
        for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

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

        //G4cout << "Created new detector region " << region->GetName() << G4endl << G4endl;
    }

    void AuxInfoReader::createVisAttributes(G4String name, const G4GDMLAuxListType* auxInfoList) {

        G4double rgba[4] = {1., 1., 1., 1.};
        G4bool visible = true;
        G4bool dauInvisible = false;
        G4bool forceWireframe = false;
        G4bool forceSolid = false;
        G4double lineWidth = 1.0;
        G4VisAttributes::LineStyle lineStyle = G4VisAttributes::unbroken;

        for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

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

        //G4cout << "Created VisAttributes " << name << G4endl << (*visAttributes) << G4endl << G4endl;
    }

    void AuxInfoReader::createDetectorHeader(G4String auxValue, const G4GDMLAuxListType* auxInfoList) {

        int detectorVersion = atoi(auxValue.c_str());

        std::string detectorName("");
        std::string author("");
        std::string description("");

        for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end(); iaux++) {

            G4String auxType = iaux->type;
            G4String auxVal = iaux->value;
            G4String auxUnit = iaux->unit;

            if (auxType == "DetectorName") {
                detectorName = auxVal;
            } else if (auxType == "Author") {
                author = auxVal;
            } else if (auxType == "Description") {
                description = auxVal;
            }
        }

        detectorHeader_ = new DetectorHeader(detectorName, detectorVersion, description, author);

        /*G4cout << G4endl;
        G4cout << "Read detector header from userinfo: " << G4endl;
        G4cout << "  DetectorName: " << detectorHeader_->getName() << G4endl;
        G4cout << "  DetectorVersion: " << detectorHeader_->getVersion() << G4endl;
        G4cout << "  Author: " << detectorHeader_->getAuthor() << G4endl;
        G4cout << "  Description: " << detectorHeader_->getDescription() << G4endl;
        G4cout << G4endl;*/

    }

}
