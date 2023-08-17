/**
 * @file APrimePhysics.cxx
 * @brief Class which defines basic APrime physics
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/APrimePhysics.h"
#include "SimCore/UserEventInformation.h"

#include "G4DarkBreM/G4APrime.h"
#include "G4DarkBreM/G4DarkBreMModel.h"

// Geant4
#include "G4Electron.hh"
#include "G4EventManager.hh"
#include "G4ProcessManager.hh"

namespace simcore {

const std::string APrimePhysics::NAME = "APrime";

/**
 * Store the atomic Z for the element in which the dark brem occurred.
 *
 * This function is registered with G4DarkBremsstrahlung and will be
 * called with the element that the dark brem will occurr off of. We
 * store the element Z in UserEventInformation for later serialization
 * into the EventHeader
 *
 * @param[in] element G4Element off-which the dark brem occurred
 */
static void store_element_z(const G4Element& element) {
  static_cast<UserEventInformation*>(
      G4EventManager::GetEventManager()->GetUserInformation()
      )->setDarkBremMaterialZ(element.GetZ());
}

APrimePhysics::APrimePhysics(const framework::config::Parameters &params)
    : G4VPhysicsConstructor(APrimePhysics::NAME),
      parameters_{params},
      process_{nullptr} {
  ap_mass_ = parameters_.getParameter<double>("ap_mass", 0.) * MeV;
  enable_ = parameters_.getParameter<bool>("enable", false);
}

void APrimePhysics::ConstructParticle() {
  /**
   * Insert A-prime into the Geant4 particle table.
   * For now we flag it as stable.
   *
   * Geant4 registers all instances derived from G4ParticleDefinition and
   * deletes them at the end of the run. We configure the A' to have
   * the input mass and the PDG ID number of 622.
   */
  G4APrime::Initialize(ap_mass_, 622);
}

void APrimePhysics::ConstructProcess() {
  // add process to electron if we are enabled
  if (enable_) {
    auto model{
        parameters_.getParameter<framework::config::Parameters>("model")};
    auto model_name{model.getParameter<std::string>("name")};
    if (model_name == "vertex_library" or model_name == "g4db") {
      static const std::map<std::string, g4db::G4DarkBreMModel::ScalingMethod>
          method_lut = {
              {"forward_only",
               g4db::G4DarkBreMModel::ScalingMethod::ForwardOnly},
              {"cm_scaling", g4db::G4DarkBreMModel::ScalingMethod::CMScaling},
              {"undefined", g4db::G4DarkBreMModel::ScalingMethod::Undefined}};
      auto scaling_method_it{
          method_lut.find(model.getParameter<std::string>("method"))};
      if (scaling_method_it == method_lut.end()) {
        EXCEPTION_RAISE(
            "BadConf",
            "Unrecognized scaling method '" +
                model.getParameter<std::string>("method") +
                "',"
                " options are 'forward_only', 'cm_scaling', or 'undefined'.");
      }
      // Note: The process variable isn't used here, but creating the
      // G4DarkBremsstahlung object has side-effects
      process_ = std::make_unique<G4DarkBremsstrahlung>(
          std::make_shared<g4db::G4DarkBreMModel>(
              model.getParameter<std::string>("library_path"),
              false /* dark brem off muons instead of electrons - we
                       always DB off electrons here */
              ,
              model.getParameter<double>("threshold"),
              model.getParameter<double>("epsilon"), scaling_method_it->second),
          parameters_.getParameter<bool>("only_one_per_event"),
          1., /* global bias - should use bias operator instead */
          parameters_.getParameter<bool>("cache_xsec"));
      process_->RegisterStorageMechanism(store_element_z);
    } else {
      EXCEPTION_RAISE("BadConf",
                      "Unrecognized model name '" + model_name + "'.");
    }
    G4cout << "[ APrimePhysics ] : Initialization of dark brem complete"
           << G4endl;
  }
}

}  // namespace simcore
