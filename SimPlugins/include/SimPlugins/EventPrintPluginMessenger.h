/**
 * @file EventPrintPluginMessenger.h
 * @brief Class that defines a macro messenger for an EventPrintPlugin
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_EVENTPRINTPLUGINMESSENGER_H_
#define SIMPLUGINS_EVENTPRINTPLUGINMESSENGER_H_

// LDMX
#include "SimPlugins/UserActionPluginMessenger.h"

// Geant4
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"

namespace sim {

/*
 * Declare the plugin class because there is a circular dep between
 * the plugin its messenger class.
 */
class EventPrintPlugin;

/**
 * @class EventPrintPluginMessenger
 * @brief Messenger class for setting parameters of the EventPrintPlugin
 */
class EventPrintPluginMessenger : public UserActionPluginMessenger {

    public:

        /**
         * Class constructor.
         * @param plugin The associated EventPrintPlugin object.
         */
        EventPrintPluginMessenger(EventPrintPlugin* plugin);

        /**
         * Class destructor.
         */
        virtual ~EventPrintPluginMessenger();

        /**
         * Process the macro command.
         * @param command The macro command.
         * @param newValue The argument values.
         */
        void SetNewValue(G4UIcommand *command, G4String newValue);

    private:

        /**
         * The associated user plugin.
         */
        EventPrintPlugin* eventPrintPlugin_;

        /**
         * Command for setting the event print modulus.
         */
        G4UIcommand* modulusCmd_;

        /**
         * Command for setting the prepend string.
         */
        G4UIcommand* prependCmd_;

        /**
         * Command for setting the append string.
         */
        G4UIcommand* appendCmd_;

        /**
         * Command for enabling start of run message.
         */
        G4UIcommand* enableStartRunCmd_;

        /**
         * Command for enabling end of run message.
         */
        G4UIcommand* enableEndRunCmd_;

        /**
         * Command for enabling start of event message.
         */
        G4UIcommand* enableStartEventCmd_;

        /**
         * Command for enabling end of event message.
         */
        G4UIcommand* enableEndEventCmd_;

        /**
         * Command for resetting the plugin state.
         */
        G4UIcommand* resetCmd_;
};


} // namespace sim

#endif
