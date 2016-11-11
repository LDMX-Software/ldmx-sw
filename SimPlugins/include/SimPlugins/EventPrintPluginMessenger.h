/**
 * @file EventPrintPluginMessenger.h
 * @brief Messenger class for setting parameters of EventPrintPlugin.
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

class EventPrintPluginMessenger : public UserActionPluginMessenger {

    public:

        EventPrintPluginMessenger(EventPrintPlugin*);

        virtual ~EventPrintPluginMessenger();

        void SetNewValue(G4UIcommand *command, G4String newValue);

    private:

        EventPrintPlugin* eventPrintPlugin_;

        G4UIcommand* modulusCmd_;
        G4UIcommand* prependCmd_;
        G4UIcommand* appendCmd_;
        G4UIcommand* enableStartRunCmd_;
        G4UIcommand* enableEndRunCmd_;
        G4UIcommand* enableStartEventCmd_;
        G4UIcommand* enableEndEventCmd_;
        G4UIcommand* resetCmd_;
};


} // namespace sim

#endif
