/*
 * @file TrackFilterMessenger.h
 * @brief Messenger class for building user track filters
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMPLUGINS_TRACKFILTERMESSENGER_H_
#define SIMPLUGINS_TRACKFILTERMESSENGER_H_

#include "SimPlugins/UserActionPluginMessenger.h"

#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithoutParameter.hh"

namespace ldmx {

    class TrackFilterPlugin;
    class TrackFilter;

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

            template<class T> T* getFilter() {
                T* filter = nullptr;
                for (auto f : filters_) {
                    if (dynamic_cast<T*>(filter)) {
                        filter = dynamic_cast<T*>(f);
                    }
                }
                if (!filter) {
                    filter = new T;
                    filters_.push_back(filter);
                }
                return filter;
            }

        private:

            TrackFilterPlugin* plugin_;

            // energy threshold
            G4UIcmdWithADoubleAndUnit* thresholdCmd_;

            // PDG ID
            G4UIcmdWithAnInteger* pdgCodeCmd_;

            // Particle name
            G4UIcmdWithAString* particleCmd_;

            // save by physics process
            G4UIcommand* processCmd_;

            // save parents from daughter process
            G4UIcommand* parentCmd_;

            // save by region name
            G4UIcommand* regionCmd_;

            // save by volume name
            G4UIcommand* volumeCmd_;

            // add a filter that always passes
            G4UIcmdWithoutParameter* passCmd_;

            // set pre or post tracking hook for the current chain
            G4UIcmdWithAString* actionCmd_;

            // create a new filter chain
            G4UIcmdWithAString* createCmd_;

            // print out filter chain information
            G4UIcmdWithAString* printCmd_;

            // current set of filters being built
            std::vector<TrackFilter*> filters_;

            // tracking action hook for the filter chain (defaults to post-tracking)
            std::string action_{"post"};
    };
}


#endif /* SIMPLUGINS_INCLUDE_SIMPLUGINS_TRACKFILTERMESSENGER_H_ */
