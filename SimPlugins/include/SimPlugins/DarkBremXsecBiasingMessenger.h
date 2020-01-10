/**
 * @file DarkBremXsecBiasingMessenger.h
 * @brief Class providing a macro messenger for a DarkBremXsecBiasingPlugin
 * @author Michael Revering, University of Minnesota
 */

#ifndef SIMPLUGINS_DarkBremXSECBIASINGPLUGINMESSENGER_H
#define SIMPLUGINS_DarkBremXSECBIASINGPLUGINMESSENGER_H

// LDMX
#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx {

// Forward declare to avoid circular dependency in headers.
    class DarkBremXsecBiasingPlugin;

    /**
     * @class DarkBremXsecBiasingMessenger
     * @brief Messenger for setting parameters on DarkBremXsecBiasingPlugin
     */
    class DarkBremXsecBiasingMessenger : UserActionPluginMessenger {

        public:

            /**
             * Class constructor.
             * @param plugin The associated DarkBremXsecBiasingPlugin object.
             */
            DarkBremXsecBiasingMessenger(DarkBremXsecBiasingPlugin* plugin);

            /**
             * Class destructor.
             */
            virtual ~DarkBremXsecBiasingMessenger() {
                delete xsecFactorCmd_;
            }

            /**
             * Process the macro command.
             * @param[in] command The macro command.
             * @param[in] newValues The argument values.
             */
            void SetNewValue(G4UIcommand *command, G4String newValue);

        private:

            /**
             * The associated DarkBremXsecBiasingPlugin object.
             */
            DarkBremXsecBiasingPlugin* biasingPlugin_;

            /**
             * The command for setting the cross-section biasing factor.
             */
            G4UIcommand* xsecFactorCmd_;

	    /**
	     * The command for changing the simulation mode.
	     */
	    G4UIcommand* modeCmd_;
    };

}

#endif
