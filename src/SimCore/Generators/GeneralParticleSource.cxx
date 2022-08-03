/**
 * @file GeneralParticleSource.cxx
 * @brief Extension of G4GeneralParticleSource.
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/Generators/GeneralParticleSource.h"

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
namespace generators {

GeneralParticleSource::GeneralParticleSource(const std::string& name,
                                             const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters) {
  init_commands_ = parameters.getParameter<std::vector<std::string>>("initCommands");
  for (const auto& cmd : init_commands_) {
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

void GeneralParticleSource::RecordConfig(const std::string& id, ldmx::RunHeader& rh) {
  rh.setStringParameter(id+" Class", "simcore::generators::GeneralParticleSource");
  std::string init_prefix{id+" Init Cmd "};
  for (std::size_t i{0}; i < init_commands_.size(); i++) {
    rh.setStringParameter(init_prefix+std::to_string(i), init_commands_.at(i));
  }
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators::GeneralParticleSource)
