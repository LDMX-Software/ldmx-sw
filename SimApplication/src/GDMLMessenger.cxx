#include "SimApplication/GDMLMessenger.h"

namespace ldmx {

    GDMLMessenger::GDMLMessenger(G4GDMLParser* parser) :
            parser_(parser) {
    }

    GDMLMessenger::~GDMLMessenger() {
        delete writeCmd_;
        delete gdmlDir_;
    }

    void GDMLMessenger::SetNewValue(G4UIcommand* cmd, G4String newValues) {
        if (cmd == writeCmd_) {
            if (parser_->GetWorldVolume()) {
                parser_->Write(newValues, parser_->GetWorldVolume(), false);
            } else {
                G4Exception("GDMLMessenger::SetNewValue", "gdml0001", JustWarning, "World volume in GDML parser was not setup!");
            }
        }
    }

}
