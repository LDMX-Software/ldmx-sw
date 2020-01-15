/**
 * @file DarkBremXsecBiasingMessenger.h
 * @brief Class providing a macro messenger for a DarkBremXsecBiasingPlugin
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMPLUGINS_DARKBREMXSECBIASINGPLUGINMESSENGER_H
#define SIMPLUGINS_DARKBREMXSECBIASINGPLUGINMESSENGER_H

// LDMX
#include "SimPlugins/UserActionPluginMessenger.h" //for inheritance
#include "SimCore/G4eDarkBremsstrahlungModel.h" //For metehod enum

namespace ldmx {

    /** Forward declare to avoid circular dependency in headers. */
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
                delete modeCmd_;
                delete madGraphDataFileCmd_;
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

            /**
             * Command to set LHE file that the dark brem mad graph data will be pulled from.
             */
            G4UIcommand* madGraphDataFileCmd_;
    };

}

#endif
