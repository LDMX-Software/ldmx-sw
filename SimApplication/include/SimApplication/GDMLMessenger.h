#ifndef SIMAPPLICATION_G4GDMLMESSENGER_H_
#define SIMAPPLICATION_G4GDMLMESSENGER_H_

#include "G4GDMLParser.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"

namespace ldmx {

    class GDMLMessenger : public G4UImessenger {

        public:

            GDMLMessenger(G4GDMLParser*);
            ~GDMLMessenger();

            void SetNewValue(G4UIcommand*, G4String);

        private:

            G4GDMLParser* parser_;
            G4UIdirectory* gdmlDir_ {new G4UIdirectory {"/ldmx/persistency/gdml/"}};
            G4UIcmdWithAString* writeCmd_ {new G4UIcmdWithAString {"/ldmx/persistency/gdml/write", this}};
    };

}

#endif
