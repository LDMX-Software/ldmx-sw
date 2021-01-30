/**
 * @file GeneralParticleSource.cxx
 * @brief Extension of G4GeneralParticleSource.
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/GeneralParticleSource.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4UImanager.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

namespace simcore {

GeneralParticleSource::GeneralParticleSource(
    const std::string& name, framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters) {
  auto initCommands{
      parameters_.getParameter<std::vector<std::string> >("initCommands")};

  for (const auto& cmd : initCommands) {
    int g4Ret = G4UImanager::GetUIpointer()->ApplyCommand(cmd);
    if (g4Ret > 0) {
      EXCEPTION_RAISE("InitCmd",
                      "Initialization command '" + cmd +
                          "' returned a failue status from Geant4: " +
                          std::to_string(g4Ret));
    }
  }
}

GeneralParticleSource::~GeneralParticleSource() {}

void GeneralParticleSource::GeneratePrimaryVertex(G4Event* event) {
  // just pass to the Geant4 implementation
  theG4Source_.GeneratePrimaryVertex(event);

  return;
}

}  // namespace simcore

DECLARE_GENERATOR(simcore, GeneralParticleSource)
