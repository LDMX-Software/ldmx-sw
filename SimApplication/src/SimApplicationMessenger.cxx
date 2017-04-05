#include "SimApplication/SimApplicationMessenger.h"

// Geant4
#include "G4ApplicationState.hh"

namespace ldmx {

    SimApplicationMessenger::SimApplicationMessenger() {
        ldmxDir_ = new G4UIdirectory("/ldmx/");
        ldmxDir_->SetGuidance("LDMX Simulation Application commands");
    }

    SimApplicationMessenger::~SimApplicationMessenger() {}

    void SimApplicationMessenger::SetNewValue(G4UIcommand*, G4String) {
    }

}
