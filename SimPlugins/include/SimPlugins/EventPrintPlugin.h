/**
 * @file EventPrintPlugin.h
 * @brief Class that defines a sim plugin to print out event information
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_EVENTPRINTPLUGIN_H_
#define SIMPLUGINS_EVENTPRINTPLUGIN_H_

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "SimPlugins/EventPrintPluginMessenger.h"

// Geant4
#include "G4UImessenger.hh"

// STL
#include <string>

namespace sim {

/**
 * @class EventPrintPlugin
 * @brief Sim plugin for printing out messages at begin and end of event and run
 */
class EventPrintPlugin : public UserActionPlugin {

    public:

        /**
         * Class constructor.
         * Creates a messenger for the class.
         */
        EventPrintPlugin() {
            _pluginMessenger = new EventPrintPluginMessenger(this);
        }

        /**
         * Class destructor.
         * Deletes the messenger for the class.
         */
        virtual ~EventPrintPlugin() {
            delete _pluginMessenger;
        }

        /**
         * Get the name of the plugin.
         */
        virtual std::string getName() {
            return "EventPrintPlugin";
        }

        /**
         * Get whether this plugin has a run action.
         * @return True to indicate this plugin has a run action.
         */
        bool hasRunAction() {
            return true;
        }

        /**
         * Get whether this plugin has an event action.
         * @return True to indicate this plugin has an event action.
         */
        bool hasEventAction() {
            return true;
        }

        /**
         * Get whether this plugin has a primary generator action.
         * @return True to indicate this plugin has a primary generator action.
         */
        bool hasPrimaryGeneratorAction() {
            return true;
        }

        /**
         * Print a start message with the run number.
         * @param aRun The current Geant4 run that is starting.
         */
        void beginRun(const G4Run* aRun) {
            if (enableStartRun_) {
                std::cout << prepend_ << " Start Run " << aRun->GetRunID() << " " << append_ << std::endl;
            }
        }

        /**
         * Print an end message with the run number.
         * @param aRun The current Geant4 run that is ending.
         */
        void endRun(const G4Run* run) {
            if (enableEndRun_) {
                std::cout << prepend_ << " End Run " << run->GetRunID() << " " << append_ << std::endl;
            }
        }

        /**
         * Print a start event message.
         * Use the primary generator hook for the start event message so it appears as early as possible in output.
         * @param anEvent The Geant4 event that is starting.
         */
        void generatePrimary(G4Event* anEvent) {
            if (enableStartEvent_) {
                if (anEvent->GetEventID() % modulus_ == 0) {
                    std::cout << prepend_ << " Start Event " << anEvent->GetEventID() << " " << append_ << std::endl;
                }
            }
        }

        /**
         * Print an end event message.
         * @param The Geant4 event that is ending.
         */
        void endEvent(const G4Event* anEvent) {
            if (enableEndEvent_) {
                if (anEvent->GetEventID() % modulus_ == 0) {
                    std::cout << prepend_ << " End Event " << anEvent->GetEventID() << " " << append_ << std::endl;
                }
            }
        }

        /**
         * Set the character string to prepend to messages.
         * @param prepend The prepending string.
         */
        void setPrepend(std::string prepend) {
            prepend_ = prepend;
        }

        /**
         * Set the character string to append to messages.
         * @param append The appending string.
         */
        void setAppend(std::string append) {
            append_ = append;
        }

        /**
         * Set the modulus which determines how often to print event messages.
         * A modulus of 1 will print every event.
         * @param modulus The event print modulus.
         */
        void setModulus(unsigned modulus) {
            modulus_ = modulus;
        }

        /**
         * Set whether a message should print at end of run.
         * @param enableEndRun True to enable end of run print out.
         */
        void setEnableEndRun(bool enableEndRun) {
            enableEndRun_ = enableEndRun;
        }

        /**
         * Set whether a message should print at start of run.
         * @param enableStartRun True to enable start of run print out.
         */
        void setEnableStartRun(bool enableStartRun) {
            enableStartRun_ = enableStartRun;
        }

        /**
         * Set whether a message should print at start of event.
         * @param enableStartEvent True to enable start of event print out.
         */
        void setEnableStartEvent(bool enableStartEvent) {
            enableStartEvent_ = enableStartEvent;
        }

        /**
         * Set whether a message should print at end of event.
         * @param enableEndEvent True to enable end of event print out.
         */
        void setEnableEndEvent(bool enableEndEvent) {
            enableEndEvent_ = enableEndEvent;
        }

        /**
         * Reset the plugin state.
         * Turns on all print outs, sets modulus to 1, and restores
         * default prepend and append strings.
         */
        void reset() {
            enableStartRun_ = true;
            enableEndRun_ = true;
            enableEndEvent_ = true;
            enableStartEvent_ = true;
            modulus_ = 1;
            prepend_ = ">>>";
            append_ = "<<<";
        }

    private:

        /**
         * The messenger for setting plugin parameters.
         */
        G4UImessenger* _pluginMessenger;

        /**
         * The event print modulus.
         */
        int modulus_{1};

        /**
         * The prepending character string.
         */
        std::string prepend_{">>>"};

        /**
         * The appending character string.
         */
        std::string append_{"<<<"};

        /**
         * Flag to enable start of run print out.
         */
        bool enableStartRun_{true};

        /**
         * Flag to enable end of run print out.
         */
        bool enableEndRun_{true};

        /**
         * Flag to enable start of event print out.
         */
        bool enableStartEvent_{true};

        /**
         * Flag to enable end of event print out.
         */
        bool enableEndEvent_{true};
};

} // namespace sim

#endif
