#include "SimCore/Geo/AuxInfoReader.h"

#include <array>

// LDMX
#include "Framework/Exception/Exception.h"
#include "SimCore/G4User/UserRegionInformation.h"
#include "SimCore/MagneticFieldMap3D.h"
#include "SimCore/MagneticFieldStore.h"
#include "SimCore/VisAttributesStore.h"

// Geant4
#include "G4FieldManager.hh"
#include "G4GDMLEvaluator.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ProductionCuts.hh"
#include "G4ProductionCutsTable.hh"
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
  delete detector_header_;
}

void AuxInfoReader::readGlobalAuxInfo() {
  const G4GDMLAuxListType* aux_info_list = parser_->GetAuxList();
  for (const auto& aux_info : *aux_info_list) {
    G4String aux_type = aux_info.type;
    G4String aux_val = aux_info.value;

    if (aux_type == "SensDet") {
      std::cerr
          << "[ WARN ] : Not defining SensDet in GDML since v1.0 of SimCore. "
             "See https://github.com/LDMX-Software/SimCore/issues/39"
          << std::endl;
    } else if (aux_type == "MagneticField") {
      createMagneticField(aux_val, aux_info.auxList);
    } else if (aux_type == "Region") {
      createRegion(aux_val, aux_info.auxList);
    } else if (aux_type == "VisAttributes") {
      createVisAttributes(aux_val, aux_info.auxList);
    } else if (aux_type == "DetectorVersion") {
      createDetectorHeader(aux_val, aux_info.auxList);
    }
  }
  return;
}

void AuxInfoReader::assignAuxInfoToVolumes() {
  const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
  std::vector<G4LogicalVolume*>::const_iterator lvciter;
  for (lvciter = lvs->begin(); lvciter != lvs->end(); lvciter++) {
    G4GDMLAuxListType aux_info_list =
        parser_->GetVolumeAuxiliaryInformation(*lvciter);

    for (const auto& aux_info : aux_info_list) {
      G4String aux_type = aux_info.type;
      G4String aux_val = aux_info.value;

      G4LogicalVolume* lv = (*lvciter);

      if (aux_type == "MagneticField") {
        const G4String& mag_field_name = aux_val;
        G4MagneticField* mag_field =
            MagneticFieldStore::getInstance()->getMagneticField(mag_field_name);
        if (mag_field != nullptr) {
          auto mgr = new G4FieldManager(mag_field);
          lv->SetFieldManager(
              mgr,
              true /* FIXME: hard-coded to force field manager to daughters */);
          // G4cout << "Assigned magnetic field " << magFieldName << " to
          // volume " << lv->GetName() << G4endl;
        } else {
          EXCEPTION_RAISE(
              "MissingInfo",
              "Unknown MagneticField ref in volume's auxiliary info: " +
                  std::string(mag_field_name.data()));
        }
      } else if (aux_type == "Region") {
        const G4String& region_name = aux_val;
        G4Region* region = G4RegionStore::GetInstance()->GetRegion(region_name);
        if (region != nullptr) {
          region->AddRootLogicalVolume(lv);
          // G4cout << "Added volume " << lv->GetName() << " to region " <<
          // regionName << G4endl;
        } else {
          EXCEPTION_RAISE("MissingInfo", "Reference region '" +
                                             std::string(region_name.data()) +
                                             "' was not found!");
        }
      } else if (aux_type == "VisAttributes") {
        const G4String& vis_name = aux_val;
        G4VisAttributes* vis_attributes =
            VisAttributesStore::getInstance()->getVisAttributes(vis_name);
        if (vis_attributes != nullptr) {
          lv->SetVisAttributes(vis_attributes);
          // G4cout << "Assigned VisAttributes " << visName << " to volume "
          // << lv->GetName() << G4endl;
        } else {
          EXCEPTION_RAISE("MissingInfo", "Referenced VisAttributes '" +
                                             std::string(vis_name.data()) +
                                             "' was not found!");
        }
      }
    }
  }
}

void AuxInfoReader::createMagneticField(const G4String& magFieldName,
                                        const G4GDMLAuxListType* auxInfoList) {
  // Find type of the mag field.
  G4String mag_field_type("");
  for (const auto& aux_info : *auxInfoList) {
    G4String aux_type = aux_info.type;
    G4String aux_val = aux_info.value;

    if (aux_type == "MagneticFieldType") {
      mag_field_type = aux_val;
      break;
    }
  }

  if (mag_field_type == "") {
    EXCEPTION_RAISE("MissingInfo",
                    "Missing MagFieldType for magnetic field definition.");
  }

  G4MagneticField* mag_field = nullptr;

  // Create a uniform mag field using the built-in Geant4 type.
  if (mag_field_type == "G4UniformMagField") {
    double bx, by, bz;
    bx = by = bz = 0.;
    for (const auto& aux_info : *auxInfoList) {
      G4String aux_type = aux_info.type;
      G4String aux_val = aux_info.value;
      G4String aux_unit = aux_info.unit;

      G4String expr = aux_val + "*" + aux_unit;
      if (aux_type == "bx") {
        bx = eval_->Evaluate(expr);
      } else if (aux_type == "by") {
        by = eval_->Evaluate(expr);
      } else if (aux_type == "bz") {
        bz = eval_->Evaluate(expr);
      }
    }
    G4ThreeVector field_components(bx, by, bz);
    mag_field = new G4UniformMagField(field_components);

    // G4cout << "Created G4UniformMagField " << magFieldName << " with field
    // components " << fieldComponents << G4endl << G4endl;

    // Create a global 3D field map by reading from a data file.
  } else if (mag_field_type == "MagneticFieldMap3D") {
    string file_name;
    double offset_x{};
    double offset_y{};
    double offset_z{};

    for (const auto& aux_info : *auxInfoList) {
      G4String aux_type = aux_info.type;
      G4String aux_val = aux_info.value;
      G4String aux_unit = aux_info.unit;

      G4String expr = aux_val + "*" + aux_unit;

      if (aux_type == "File") {
        file_name = aux_val;
      } else if (aux_type == "OffsetX") {
        offset_x = eval_->Evaluate(expr);
      } else if (aux_type == "OffsetY") {
        offset_y = eval_->Evaluate(expr);
      } else if (aux_type == "OffsetZ") {
        offset_z = eval_->Evaluate(expr);
      }
    }

    if (file_name.size() == 0) {
      EXCEPTION_RAISE("MissingInfo",
                      "File info with field data was not provided.");
    }

    // Create new 3D field map.
    mag_field =
        new MagneticFieldMap3D(file_name.c_str(), offset_x, offset_y, offset_z);

    // Assign field map as global field.
    G4FieldManager* field_mgr =
        G4TransportationManager::GetTransportationManager()->GetFieldManager();
    if (field_mgr->GetDetectorField() != nullptr) {
      EXCEPTION_RAISE("MisAssign", "Global mag field was already assigned.");
    }
    field_mgr->SetDetectorField(mag_field);
    field_mgr->CreateChordFinder(mag_field);

  } else {
    EXCEPTION_RAISE("UnknownType", "Unknown MagFieldType '" +
                                       std::string(mag_field_type.data()) +
                                       "' in auxiliary info.");
  }

  MagneticFieldStore::getInstance()->addMagneticField(magFieldName, mag_field);
}

void AuxInfoReader::createRegion(const G4String& name,
                                 const G4GDMLAuxListType* auxInfoList) {
  bool store_trajectories = true;
  for (const auto& aux_info : *auxInfoList) {
    G4String aux_type = aux_info.type;
    G4String aux_val = aux_info.value;

    if (aux_type == "StoreTrajectories") {
      if (aux_val == "false") {
        store_trajectories = false;
      } else if (aux_val == "true") {
        store_trajectories = true;
      }
    }
  }
  G4VUserRegionInformation* region_info =
      new UserRegionInformation(store_trajectories);
  // This looks like a memory leak, but isn't. I (Einar) have checked. Geant4
  // registers the region in the constructor and deletes it at the end.
  //
  // Some static analysis tools may struggle with identifying that this one
  // happens to be fine. The NOLINT comment tells clang-tidy to not bother
  // within the region
  //
  // NOLINTBEGIN
  auto region = new G4Region(name);
  region->SetUserInformation(region_info);
  // To get rid of those pesky G4 warnings
  region->SetProductionCuts(G4ProductionCutsTable::GetProductionCutsTable()
                                ->GetDefaultProductionCuts());
}
// NOLINTEND

void AuxInfoReader::createVisAttributes(const G4String& name,
                                        const G4GDMLAuxListType* auxInfoList) {
  std::array<G4double, 4> rgba = {1., 1., 1., 1.};
  G4bool visible = true;
  G4bool dau_invisible = false;
  G4bool force_wireframe = false;
  G4bool force_solid = false;
  G4double line_width = 1.0;
  G4VisAttributes::LineStyle line_style = G4VisAttributes::unbroken;

  for (const auto& aux_info : *auxInfoList) {
    G4String aux_type = aux_info.type;
    G4String aux_val = aux_info.value;

    if (aux_type == "R") {
      rgba[0] = atof(aux_val.c_str());
    } else if (aux_type == "G") {
      rgba[1] = atof(aux_val.c_str());
    } else if (aux_type == "B") {
      rgba[2] = atof(aux_val.c_str());
    } else if (aux_type == "A") {
      rgba[3] = atof(aux_val.c_str());
    } else if (aux_type == "Style") {
      if (aux_val == "wireframe") {
        force_wireframe = true;
      } else if (aux_val == "solid") {
        force_solid = true;
      }
    } else if (aux_type == "DaughtersInvisible") {
      if (aux_val == "true") {
        dau_invisible = true;
      } else if (aux_val == "false") {
        dau_invisible = false;
      }
    } else if (aux_type == "Visible") {
      if (aux_val == "true") {
        visible = true;
      } else if (aux_val == "false") {
        visible = false;
      }
    } else if (aux_type == "LineStyle") {
      if (aux_val == "unbroken") {
        line_style = G4VisAttributes::unbroken;
      } else if (aux_val == "dashed") {
        line_style = G4VisAttributes::dashed;
      } else if (aux_val == "dotted") {
        line_style = G4VisAttributes::dotted;
      }
    } else if (aux_type == "LineWidth") {
      line_width = atof(aux_val.c_str());
    }
  }

  auto vis_attributes = new G4VisAttributes();
  vis_attributes->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
  vis_attributes->SetVisibility(visible);
  vis_attributes->SetDaughtersInvisible(dau_invisible);
  vis_attributes->SetForceWireframe(force_wireframe);
  vis_attributes->SetForceSolid(force_solid);
  vis_attributes->SetLineWidth(line_width);
  vis_attributes->SetLineStyle(line_style);
  VisAttributesStore::getInstance()->addVisAttributes(name, vis_attributes);

  // G4cout << "Created VisAttributes " << name << G4endl << (*visAttributes) <<
  // G4endl << G4endl;
}

void AuxInfoReader::createDetectorHeader(const G4String& auxValue,
                                         const G4GDMLAuxListType* auxInfoList) {
  int detector_version = atoi(auxValue.c_str());

  std::string detector_name("");
  std::string author("");
  std::string description("");

  for (const auto& aux_info : *auxInfoList) {
    G4String aux_type = aux_info.type;
    G4String aux_val = aux_info.value;

    if (aux_type == "DetectorName") {
      detector_name = aux_val;
    } else if (aux_type == "Author") {
      author = aux_val;
    } else if (aux_type == "Description") {
      description = aux_val;
    }
  }

  detector_header_ = new ldmx::DetectorHeader(detector_name, detector_version,
                                              description, author);

  /*G4cout << G4endl;
  G4cout << "Read detector header from userinfo: " << G4endl;
  G4cout << "  DetectorName: " << detector_header_->getName() << G4endl;
  G4cout << "  DetectorVersion: " << detector_header_->getVersion() << G4endl;
  G4cout << "  Author: " << detector_header_->getAuthor() << G4endl;
  G4cout << "  Description: " << detector_header_->getDescription() << G4endl;
  G4cout << G4endl;*/
}
}  // namespace simcore::geo
