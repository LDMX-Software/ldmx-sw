/**
 * @file EventPrintPlugin.h
 * @brief Sim plugin to print out event and run IDs during a run.
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

class EventPrintPlugin : public UserActionPlugin {

    public:

        EventPrintPlugin() {
            _pluginMessenger = new EventPrintPluginMessenger(this);
        }

        virtual ~EventPrintPlugin() {
            delete _pluginMessenger;
        }

        virtual std::string getName() {
            return "EventPrintPlugin";
        }

        bool hasRunAction() {
            return true;
        }

        bool hasEventAction() {
            return true;
        }

        bool hasPrimaryGeneratorAction() {
            return true;
        }

        void beginRun(const G4Run* run) {
            if (enableStartRun_) {
                std::cout << prepend_ << " Start Run " << run->GetRunID() << " " << append_ << std::endl;
            }
        }

        void endRun(const G4Run* run) {
            if (enableEndRun_) {
                std::cout << prepend_ << " End Run " << run->GetRunID() << " " << append_ << std::endl;
            }
        }

        /**
         * Use the primary generator hook for the start event message so it appears as early as possible in output.
         */
        void generatePrimary(G4Event* event) {
            if (enableStartEvent_) {
                if (event->GetEventID() % modulus_ == 0) {
                    std::cout << prepend_ << " Start Event " << event->GetEventID() << " " << append_ << std::endl;
                }
            }
        }

        void endEvent(const G4Event* event) {
            if (enableEndEvent_) {
                if (event->GetEventID() % modulus_ == 0) {
                    std::cout << prepend_ << " End Event " << event->GetEventID() << " " << append_ << std::endl;
                }
            }
        }

        void setPrepend(std::string prepend) {
            prepend_ = prepend;
        }

        void setAppend(std::string append) {
            append_ = append;
        }

        void setModulus(unsigned modulus) {
            modulus_ = modulus;
        }

        void setEnableEndRun(bool enableEndRun) {
            enableEndRun_ = enableEndRun;
        }

        void setEnableStartRun(bool enableStartRun) {
            enableStartRun_ = enableStartRun;
        }

        void setEnableStartEvent(bool enableStartEvent) {
            enableStartEvent_ = enableStartEvent;
        }

        void setEnableEndEvent(bool enableEndEvent) {
            enableEndEvent_ = enableEndEvent;
        }

        /**
         * Turns on all print outs, sets modulus to 1, and restores
         * default prepend and append strings.
         */
        void reset() {
            enableStartRun_ = true;
            enableEndRun_ = true;
            enableEndEvent_ = true;
            enableStartEvent_ = true;
            modulus_ = 1;
            prepend_ = ">>>>";
            append_ = "<<<<";
        }

    private:

        G4UImessenger* _pluginMessenger;

        int modulus_{1};
        std::string prepend_{">>>"};
        std::string append_{"<<<"};
        bool enableStartRun_{true};
        bool enableEndRun_{true};
        bool enableStartEvent_{true};
        bool enableEndEvent_{true};
};

} // namespace sim

#endif
