/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Exception/Exception.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/BiasOperators/XsecBiasingOperator.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/G4User/VolumeChecks.h"
#include "SimCore/SDs/SensitiveDetector.h"

namespace simcore {
namespace logical_volume_tests {

/**
 * Define the type of all these functional tests.
 *
 * Used below when determining which test to use.
 */
using Test = bool (*)(G4LogicalVolume*, const std::string&);

}  // namespace logical_volume_tests

DetectorConstruction::DetectorConstruction(
    std::shared_ptr<simcore::geo::Parser> parser,
    framework::config::Parameters& parameters, ConditionsInterface& ci)
    : parser_(parser), parameters_{parameters}, conditions_interface_{ci} {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
  return parser_->getWorldVolume();
}

void DetectorConstruction::ConstructSDandField() {
  auto sens_dets{parameters_.get<std::vector<framework::config::Parameters>>(
      "sensitive_detectors", {})};
  for (auto& det : sens_dets) {
    // create
    auto sd = SensitiveDetector::Factory::get().make(
        det.get<std::string>("class_name"),
        det.get<std::string>("instance_name"), conditions_interface_, det);
    if (not sd) {
      EXCEPTION_RAISE("UnableToCreate",
                      "Unable to create a SensitiveDetector of type " +
                          det.get<std::string>("class_name"));
    }
    // attach to volumes
    for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
      if (sd.value()->isSensDet(volume)) {
        ldmx_log(debug) << "Attaching " << sd.value()->GetName() << " to "
                        << volume->GetName();
        volume->SetSensitiveDetector(sd.value());
      }
    }
  }

  // Biasing operators were created in RunManager::setupPhysics
  //  which is called before G4RunManager::Initialize
  //  which is where this method ends up being called.
  simcore::XsecBiasingOperator::Factory::get().apply([&](auto bop) {
    logical_volume_tests::Test include_volume_test{nullptr};
    if (bop->getVolumeToBias().compare("ecal") == 0) {
      include_volume_test = &simcore::g4user::volumechecks::isInEcal;
    } else if (bop->getVolumeToBias().compare("old_ecal") == 0) {
      include_volume_test = &simcore::g4user::volumechecks::isInEcalOld;
    } else if (bop->getVolumeToBias().compare("target") == 0) {
      include_volume_test = &simcore::g4user::volumechecks::isInTargetOnly;
    } else if (bop->getVolumeToBias().compare("target_region") == 0) {
      include_volume_test = &simcore::g4user::volumechecks::isInTargetRegion;
    } else if (bop->getVolumeToBias().compare("hcal") == 0) {
      include_volume_test = &simcore::g4user::volumechecks::isInHcal;
    } else {
      std::cerr << "[ DetectorConstruction ] : "
                << "WARN - Requested volume to bias '" << bop->getVolumeToBias()
                << "' is not recognized. Will attach volumes based on if their"
                << " name contains the volume to bias." << std::endl;
      include_volume_test = &simcore::g4user::volumechecks::nameContains;
    }

    for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
      auto volume_name = volume->GetName();
      if (include_volume_test(volume, bop->getVolumeToBias())) {
        bop->AttachTo(volume);
        ldmx_log(debug) << "Attaching biasing operator " << bop->GetName()
                        << " to volume " << volume->GetName();
      }  // BOP attached to target or ecal
    }  // loop over volumes
  });  // loop over biasing operators
}
}  // namespace simcore
