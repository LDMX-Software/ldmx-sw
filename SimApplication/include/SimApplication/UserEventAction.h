/**
 * @file UserEventAction.h
 * @brief Class which implements the Geant4 user event action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USEREVENTACTION_H_
#define SIMAPPLICATION_USEREVENTACTION_H_

// LDMX
#include "SimApplication/SimParticleBuilder.h"
#include "SimPlugins/PluginManagerAccessor.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"

// Geant4
#include "G4UserEventAction.hh"
#include "G4Event.hh"

namespace ldmx {

    /**
     * @class UserEventAction
     * @brief Implementation of user event action hook
     */
    class UserEventAction : public G4UserEventAction, public PluginManagerAccessor {

        public:

            /**
             * Class constructor.
             */
            UserEventAction() {
            }

            /**
             * Class destructor.
             */
            virtual ~UserEventAction() {
            }

            /**
             * Implementation of begin of event hook.
             * @param anEvent The Geant4 event.
             */
            void BeginOfEventAction(const G4Event* anEvent);

            /**
             * Implementation of end of event hook.
             * @param anEvent The Geant4 event.
             */
            void EndOfEventAction(const G4Event* anEvent);

    };

}

#endif
