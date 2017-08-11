#ifndef SIMAPPLICATION_G4GDMLMESSENGER_H_
#define SIMAPPLICATION_G4GDMLMESSENGER_H_

#include "G4GDMLParser.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"

namespace ldmx {

    class GDMLReadStructure;

    class GDMLMessenger : public G4UImessenger {

        public:

            GDMLMessenger(G4GDMLParser*, GDMLReadStructure*);

            ~GDMLMessenger();

            void SetNewValue(G4UIcommand*, G4String);

        private:

            G4GDMLParser* parser_;
            GDMLReadStructure* gdmlRead_;
            G4UIdirectory* gdmlDir_ {new G4UIdirectory {"/ldmx/persistency/gdml/"}};
            G4UIcmdWithAString* writeCmd_ {new G4UIcmdWithAString {"/ldmx/persistency/gdml/write", this}};
            G4UIcmdWithAString* readCmd_ {new G4UIcmdWithAString {"/ldmx/persistency/gdml/read", this}};
            G4UIcmdWithAString* removeModuleCmd_ {new G4UIcmdWithAString {"/ldmx/persistency/gdml/removeModule", this}};
    };

}

#endif
