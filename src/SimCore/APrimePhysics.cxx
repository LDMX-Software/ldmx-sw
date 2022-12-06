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
   * deletes them at the end of the run.
   */
  G4APrime::Initialize(ap_mass_);
}

void APrimePhysics::ConstructProcess() {
  // add process to electron if we are enabled
  if (enable_) {
    auto model{parameters_.getParameter<framework::config::Parameters>("model")};
    auto model_name{model.getParameter<std::string>("name")};
    if (model_name == "vertex_library" or model_name == "g4db") {
      auto proc = new G4DarkBremsstrahlung(
        std::make_shared<g4db::G4DarkBreMModel>(
              model.getParameter<std::string>("method"),
              model.getParameter<double>("threshold"),
              model.getParameter<double>("epsilon"),
              model.getParameter<std::string>("library_path"),
              false /* dark brem off muons instead of electrons */),
          parameters_.getParameter<bool>("only_one_per_event"),
          parameters_.getParameter<double>("global_bias"),
          parameters_.getParameter<bool>("cache_xsec", true));
    } else {
      EXCEPTION_RAISE("BadConf",
          "Unrecognized model name '"+model_name+"'.");
    }
    G4cout << "[ APrimePhysics ] : Initialization complete" << G4endl;
  }
}

}  // namespace simcore
