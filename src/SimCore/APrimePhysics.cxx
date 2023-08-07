/**
 * @file APrimePhysics.cxx
 * @brief Class which defines basic APrime physics
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/APrimePhysics.h"

#include "G4DarkBreM/G4APrime.h"
#include "G4DarkBreM/G4DarkBreMModel.h"

// Geant4
#include "G4Electron.hh"
#include "G4ProcessManager.hh"

namespace simcore {

const std::string APrimePhysics::NAME = "APrime";

APrimePhysics::APrimePhysics(const framework::config::Parameters &params)
    : G4VPhysicsConstructor(APrimePhysics::NAME), parameters_{params} {
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
      // Note: The proc variable isn't used here, but creating the
      // G4DarkBremsstahlung object has side-effects
      [[maybe_unused]] auto proc = new G4DarkBremsstrahlung(
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
    } else {
      EXCEPTION_RAISE("BadConf",
                      "Unrecognized model name '" + model_name + "'.");
    }
    G4cout << "[ APrimePhysics ] : Initialization of dark brem complete"
           << G4endl;
  }
}

}  // namespace simcore
