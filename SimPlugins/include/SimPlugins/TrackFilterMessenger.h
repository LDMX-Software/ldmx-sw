/*
 * @file TrackFilterMessenger.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMPLUGINS_TRACKFILTERMESSENGER_H_
#define SIMPLUGINS_TRACKFILTERMESSENGER_H_

#include "SimCore/TrackFilter.h"
#include "SimPlugins/UserActionPluginMessenger.h"

#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"

namespace ldmx {

    class TrackFilterPlugin;

    class TrackFilterMessenger : public UserActionPluginMessenger {

        public:

            TrackFilterMessenger(TrackFilterPlugin* plugin);

            virtual ~TrackFilterMessenger() {;}

            /**
             * Handle a messenger command.
             * @param command The command being handled.
             * @param newValue The parameter values.
             */
            void SetNewValue(G4UIcommand *command, G4String newValue);

        private:

            TrackFilterPlugin* plugin_;

            // energy threshold
            G4UIcmdWithADoubleAndUnit* thresholdCmd_;

            // PDG ID
            G4UIcmdWithAnInteger* pdgCodeCmd_;

            // save by physics process
            G4UIcommand* processCmd_;

            // save parents from daughter process
            G4UIcommand* parentCmd_;

            // create a new filter chain
            G4UIcmdWithAString* createCmd_;

            // current set of filters being built
            //TrackFilterChain* filters_{new TrackFilterChain};
            std::vector<TrackFilter*> filters_;
    };
}


#endif /* SIMPLUGINS_INCLUDE_SIMPLUGINS_TRACKFILTERMESSENGER_H_ */
