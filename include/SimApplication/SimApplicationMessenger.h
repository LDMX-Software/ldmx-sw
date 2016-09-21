#ifndef SIMAPPLICATION_SIMAPPLICATIONMESSENGER_H_
#define SIMAPPLICATION_SIMAPPLICATIONMESSENGER_H_ 1

// Geant4
#include "G4UImessenger.hh"

class SimApplicationMessenger : public G4UImessenger {

    public:

        /**
         * Class constructor.
         */
        SimApplicationMessenger();

        /**
         * Class destructor.
         */
        virtual ~SimApplicationMessenger();

    public:

        /**
         * Process the macro command.
         * @param[in] command The macro command.
         * @param[in] newValues The argument values.
         */
        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        G4UIdirectory* ldmxDir;
        G4UIdirectory* persistencyDir;
        G4UIdirectory* rootDir;

        G4UIcommand* rootFileCmd;
};

#endif
