#include "SimCore/DetectorConstruction.h"

#include "Framework/Exception/Exception.h"
#include "SimCore/XsecBiasingOperator.h"
#include "SimCore/SensitiveDetector.h"

namespace simcore {

namespace logical_volume_tests {

/**
 * isInEcal
 *
 * Check that the passed volume is inside the ECal
 *
 * @TODO this is _horrible_
 * can we get an 'ecal' and 'hcal' region instead
 * of just a 'CalorimeterRegion' region?
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
static bool isInEcal(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  G4String volumeName = vol->GetName();
  return ((volumeName.contains("Si") || volumeName.contains("W") ||
           volumeName.contains("PCB") || volumeName.contains("strongback") ||
           volumeName.contains("Glue") || volumeName.contains("CFMix") ||
           volumeName.contains("Al") || volumeName.contains("C")
          ) && volumeName.contains("volume"));
}

/**
 * isInHcal
 *
 * Check that the passed volume is inside the HCal
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
static bool isInHcal(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  G4String volumeName = vol->GetName();
  return ((volumeName.contains("abso2") || volumeName.contains("abso3") ||
           volumeName.contains("ScintBox") || volumeName.contains("absoBox")) &&
          volumeName.contains("volume"));
}

/**
 * isInEcalOld
 *
 * This is the old method for checking if the passed volume was inside the ECal
 * and only looks for tungsten or silicon layers.
 *
 * @note Deprecating soon (hopefully).
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
static bool isInEcalOld(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  G4String volumeName = vol->GetName();
  return ((volumeName.contains("Si") || volumeName.contains("W")) &&
          volumeName.contains("volume"));
}

/**
 * isInTargetRegion
 *
 * Check if the passed volume is inside the target region.
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
static bool isInTargetRegion(G4LogicalVolume* vol,
                             const std::string& vol_to_bias) {
  auto region = vol->GetRegion();
  return (region and region->GetName().contains("target"));
}

/**
 * isInTargetRegion
 *
 * Check if the passed volume is inside the target volume.
 *
 * @note This leaves out the trig scint modules inside the target region.
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias UNUSED name of volume to bias
 */
static bool isInTargetOnly(G4LogicalVolume* vol,
                           const std::string& vol_to_bias) {
  return vol->GetName().contains("target");
}

/**
 * nameContains
 *
 * Check if the passed volume has a name containing the
 * name of the volume to bias.
 *
 * @note This is the default if we don't recognize
 * the volume to bias that is requested.
 *
 * @param[in] vol G4LogicalVolume to check
 * @param[in] vol_to_bias name of volume to bias
 */
static bool nameContains(G4LogicalVolume* vol, const std::string& vol_to_bias) {
  return vol->GetName().contains(vol_to_bias);
}

/**
 * Define the type of all these functional tests.
 *
 * Used below when determining which test to use.
 */
typedef bool (*Test)(G4LogicalVolume*, const std::string&);

}  // namespace logical_volume_tests

DetectorConstruction::DetectorConstruction(
    simcore::geo::Parser* parser, framework::config::Parameters& parameters,
    ConditionsInterface& ci)
    : parser_(parser), parameters_{parameters}, conditions_interface_{ci} {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
  return parser_->GetWorldVolume();
}

void DetectorConstruction::ConstructSDandField() {
  auto sens_dets{parameters_.getParameter<std::vector<framework::config::Parameters>>(
      "sensitive_detectors",{})};
  for (auto& det : sens_dets) {
    // create
    auto sd = SensitiveDetector::Factory::get().make(
        det.getParameter<std::string>("class_name"),
        det.getParameter<std::string>("instance_name"),
        conditions_interface_, det);
    // attach to volumes
    for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
      if (sd->isSensDet(volume)) {
        std::cout << "[ DetectorConstruction ] : "
          << "Attaching " << sd->GetName()
          << " to " << volume->GetName() << std::endl;
        volume->SetSensitiveDetector(sd);
      }
    }
  }

  // Biasing operators were created in RunManager::setupPhysics
  //  which is called before G4RunManager::Initialize
  //  which is where this method ends up being called.
  simcore::XsecBiasingOperator::Factory::get().apply([](auto bop) {
    logical_volume_tests::Test includeVolumeTest{nullptr};
    if (bop->getVolumeToBias().compare("ecal") == 0) {
      includeVolumeTest = &logical_volume_tests::isInEcal;
    } else if (bop->getVolumeToBias().compare("old_ecal") == 0) {
      includeVolumeTest = &logical_volume_tests::isInEcalOld;
    } else if (bop->getVolumeToBias().compare("target") == 0) {
      includeVolumeTest = &logical_volume_tests::isInTargetOnly;
    } else if (bop->getVolumeToBias().compare("target_region") == 0) {
      includeVolumeTest = &logical_volume_tests::isInTargetRegion;
    } else if (bop->getVolumeToBias().compare("hcal") == 0) {
      includeVolumeTest = &logical_volume_tests::isInHcal;
    } else {
      std::cerr << "[ DetectorConstruction ] : "
                << "WARN - Requested volume to bias '" << bop->getVolumeToBias()
                << "' is not recognized. Will attach volumes based on if their"
                << " name contains the volume to bias." << std::endl;
      includeVolumeTest = &logical_volume_tests::nameContains;
    }

    for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
      auto volume_name = volume->GetName();
      if (includeVolumeTest(volume, bop->getVolumeToBias())) {
        bop->AttachTo(volume);
        std::cout << "[ DetectorConstruction ]: "
                  << "Attaching biasing operator " << bop->GetName()
                  << " to volume " << volume->GetName() << std::endl;
      }  // BOP attached to target or ecal
    }    // loop over volumes
  });    // loop over biasing operators
}
}  // namespace simcore
