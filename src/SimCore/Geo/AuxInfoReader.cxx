#include "SimCore/Geo/AuxInfoReader.h"

// LDMX
#include "Framework/Exception/Exception.h"
#include "SimCore/MagneticFieldMap3D.h"
#include "SimCore/MagneticFieldStore.h"
#include "SimCore/UserRegionInformation.h"
#include "SimCore/VisAttributesStore.h"

// Geant4
#include "G4FieldManager.hh"
#include "G4GDMLEvaluator.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UniformMagField.hh"

// STL
#include <cstdlib>
#include <string>

using std::string;

namespace simcore::geo {

AuxInfoReader::AuxInfoReader(G4GDMLParser* theParser,
                             const framework::config::Parameters& ps)
    : parser_(theParser), eval_(new G4GDMLEvaluator) {}

AuxInfoReader::~AuxInfoReader() {
  delete eval_;
  delete detectorHeader_;
}

void AuxInfoReader::readGlobalAuxInfo() {
  const G4GDMLAuxListType* auxInfoList = parser_->GetAuxList();
  for (const auto& auxInfo : *auxInfoList) {
    G4String auxType = auxInfo.type;
    G4String auxVal = auxInfo.value;

    if (auxType == "SensDet") {
      std::cerr
          << "[ WARN ] : Not defining SensDet in GDML since v1.0 of SimCore. "
             "See https://github.com/LDMX-Software/SimCore/issues/39"
          << std::endl;
    } else if (auxType == "MagneticField") {
      createMagneticField(auxVal, auxInfo.auxList);
    } else if (auxType == "Region") {
      createRegion(auxVal, auxInfo.auxList);
    } else if (auxType == "VisAttributes") {
      createVisAttributes(auxVal, auxInfo.auxList);
    } else if (auxType == "DetectorVersion") {
      createDetectorHeader(auxVal, auxInfo.auxList);
    }
  }
  return;
}

void AuxInfoReader::assignAuxInfoToVolumes() {
  const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
  std::vector<G4LogicalVolume*>::const_iterator lvciter;
  for (lvciter = lvs->begin(); lvciter != lvs->end(); lvciter++) {
    G4GDMLAuxListType auxInfoList =
        parser_->GetVolumeAuxiliaryInformation(*lvciter);

    for (const auto& auxInfo : auxInfoList) {
      G4String auxType = auxInfo.type;
      G4String auxVal = auxInfo.value;

      G4LogicalVolume* lv = (*lvciter);

      if (auxType == "MagneticField") {
        const G4String& magFieldName = auxVal;
        G4MagneticField* magField =
            MagneticFieldStore::getInstance()->getMagneticField(magFieldName);
        if (magField != nullptr) {
          auto mgr = new G4FieldManager(magField);
          lv->SetFieldManager(
              mgr,
              true /* FIXME: hard-coded to force field manager to daughters */);
          // G4cout << "Assigned magnetic field " << magFieldName << " to
          // volume " << lv->GetName() << G4endl;
        } else {
          EXCEPTION_RAISE(
              "MissingInfo",
              "Unknown MagneticField ref in volume's auxiliary info: " +
                  std::string(magFieldName.data()));
        }
      } else if (auxType == "Region") {
        const G4String& regionName = auxVal;
        G4Region* region = G4RegionStore::GetInstance()->GetRegion(regionName);
        if (region != nullptr) {
          region->AddRootLogicalVolume(lv);
          // G4cout << "Added volume " << lv->GetName() << " to region " <<
          // regionName << G4endl;
        } else {
          EXCEPTION_RAISE("MissingInfo", "Reference region '" +
                                             std::string(regionName.data()) +
                                             "' was not found!");
        }
      } else if (auxType == "VisAttributes") {
        const G4String& visName = auxVal;
        G4VisAttributes* visAttributes =
            VisAttributesStore::getInstance()->getVisAttributes(visName);
        if (visAttributes != nullptr) {
          lv->SetVisAttributes(visAttributes);
          // G4cout << "Assigned VisAttributes " << visName << " to volume "
          // << lv->GetName() << G4endl;
        } else {
          EXCEPTION_RAISE("MissingInfo", "Referenced VisAttributes '" +
                                             std::string(visName.data()) +
                                             "' was not found!");
        }
      }
    }
  }
}

void AuxInfoReader::createMagneticField(const G4String& magFieldName,
                                        const G4GDMLAuxListType* auxInfoList) {
  // Find type of the mag field.
  G4String magFieldType("");
  for (const auto& auxInfo : *auxInfoList) {
    G4String auxType = auxInfo.type;
    G4String auxVal = auxInfo.value;

    if (auxType == "MagneticFieldType") {
      magFieldType = auxVal;
      break;
    }
  }

  if (magFieldType == "") {
    EXCEPTION_RAISE("MissingInfo",
                    "Missing MagFieldType for magnetic field definition.");
  }

  G4MagneticField* magField = nullptr;

  // Create a uniform mag field using the built-in Geant4 type.
  if (magFieldType == "G4UniformMagField") {
    double bx, by, bz;
    bx = by = bz = 0.;
    for (const auto& auxInfo : *auxInfoList) {
      G4String auxType = auxInfo.type;
      G4String auxVal = auxInfo.value;
      G4String auxUnit = auxInfo.unit;

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

    // G4cout << "Created G4UniformMagField " << magFieldName << " with field
    // components " << fieldComponents << G4endl << G4endl;

    // Create a global 3D field map by reading from a data file.
  } else if (magFieldType == "MagneticFieldMap3D") {
    string fileName;
    double offsetX{};
    double offsetY{};
    double offsetZ{};

    for (const auto& auxInfo : *auxInfoList) {
      G4String auxType = auxInfo.type;
      G4String auxVal = auxInfo.value;
      G4String auxUnit = auxInfo.unit;

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
      EXCEPTION_RAISE("MissingInfo",
                      "File info with field data was not provided.");
    }

    // Create new 3D field map.
    magField =
        new MagneticFieldMap3D(fileName.c_str(), offsetX, offsetY, offsetZ);

    // Assign field map as global field.
    G4FieldManager* fieldMgr =
        G4TransportationManager::GetTransportationManager()->GetFieldManager();
    if (fieldMgr->GetDetectorField() != nullptr) {
      EXCEPTION_RAISE("MisAssign", "Global mag field was already assigned.");
    }
    fieldMgr->SetDetectorField(magField);
    fieldMgr->CreateChordFinder(magField);

  } else {
    EXCEPTION_RAISE("UnknownType", "Unknown MagFieldType '" +
                                       std::string(magFieldType.data()) +
                                       "' in auxiliary info.");
  }

  MagneticFieldStore::getInstance()->addMagneticField(magFieldName, magField);
}

void AuxInfoReader::createRegion(const G4String& name,
                                 const G4GDMLAuxListType* auxInfoList) {
  bool storeTrajectories = true;
  for (const auto& auxInfo : *auxInfoList) {
    G4String auxType = auxInfo.type;
    G4String auxVal = auxInfo.value;

    if (auxType == "StoreTrajectories") {
      if (auxVal == "false") {
        storeTrajectories = false;
      } else if (auxVal == "true") {
        storeTrajectories = true;
      }
    }
  }
  G4VUserRegionInformation* regionInfo =
      new UserRegionInformation(storeTrajectories);
  // This looks like a memory leak, but isn't. I (Einar) have checked. Geant4
  // registers the region in the constructor and deletes it at the end.
  //
  // Some static analysis tools may struggle with identifying that this one
  // happens to be fine. The NOLINT comment tells clang-tidy to not bother
  // within the region
  //
  // NOLINTBEGIN
  auto region = new G4Region(name);
  region->SetUserInformation(regionInfo);
}
// NOLINTEND

void AuxInfoReader::createVisAttributes(const G4String& name,
                                        const G4GDMLAuxListType* auxInfoList) {
  std::array<G4double, 4> rgba = {1., 1., 1., 1.};
  G4bool visible = true;
  G4bool dauInvisible = false;
  G4bool forceWireframe = false;
  G4bool forceSolid = false;
  G4double lineWidth = 1.0;
  G4VisAttributes::LineStyle lineStyle = G4VisAttributes::unbroken;

  for (const auto& auxInfo : *auxInfoList) {
    G4String auxType = auxInfo.type;
    G4String auxVal = auxInfo.value;

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

  auto visAttributes = new G4VisAttributes();
  visAttributes->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
  visAttributes->SetVisibility(visible);
  visAttributes->SetDaughtersInvisible(dauInvisible);
  visAttributes->SetForceWireframe(forceWireframe);
  visAttributes->SetForceSolid(forceSolid);
  visAttributes->SetLineWidth(lineWidth);
  visAttributes->SetLineStyle(lineStyle);
  VisAttributesStore::getInstance()->addVisAttributes(name, visAttributes);

  // G4cout << "Created VisAttributes " << name << G4endl << (*visAttributes) <<
  // G4endl << G4endl;
}

void AuxInfoReader::createDetectorHeader(const G4String& auxValue,
                                         const G4GDMLAuxListType* auxInfoList) {
  int detectorVersion = atoi(auxValue.c_str());

  std::string detectorName("");
  std::string author("");
  std::string description("");

  for (const auto& auxInfo : *auxInfoList) {
    G4String auxType = auxInfo.type;
    G4String auxVal = auxInfo.value;

    if (auxType == "DetectorName") {
      detectorName = auxVal;
    } else if (auxType == "Author") {
      author = auxVal;
    } else if (auxType == "Description") {
      description = auxVal;
    }
  }

  detectorHeader_ = new ldmx::DetectorHeader(detectorName, detectorVersion,
                                             description, author);

  /*G4cout << G4endl;
  G4cout << "Read detector header from userinfo: " << G4endl;
  G4cout << "  DetectorName: " << detectorHeader_->getName() << G4endl;
  G4cout << "  DetectorVersion: " << detectorHeader_->getVersion() << G4endl;
  G4cout << "  Author: " << detectorHeader_->getAuthor() << G4endl;
  G4cout << "  Description: " << detectorHeader_->getDescription() << G4endl;
  G4cout << G4endl;*/
}
}  // namespace simcore::geo
