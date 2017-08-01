/**
 * @file SimpleProcessFilter.h
 * @brief Class defining a UserActionPlugin that filters out events if a
 *        particle doesn't interact via a specified process.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_SIMPLEPROCESSFILTER_H_
#define BIASING_SIMPLEPROCESSFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"
#include "Biasing/SimpleProcessFilterMessenger.h"

namespace ldmx {

    class SimpleProcessFilterMessenger;  

    class SimpleProcessFilter : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            SimpleProcessFilter();

            /**
             * Class destructor.
             */
            ~SimpleProcessFilter();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "SimpleProcessFilter";
            }

            /**
             * Get whether this plugin implements the stepping action.
             * @return True to indicate this plugin implements the stepping action.
             */
            bool hasSteppingAction() {
                return true;
            }

            /**
             * Implementmthe stepping action which performs the target volume biasing.
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step);

            /** Set the parent ID of the particle to which the filter will be applied to. */
            void setParentID(int parentID) { parentID_ = parentID; };
            
            /** Set the track ID of the particle to which the filter will be applied to. */
            void setTrackID(int trackID) { trackID_ = trackID; };

            /** Set the PDG ID of the particle to which the filter will be applied to. */
            void setPdgID(int pdgID) { pdgID_ = pdgID; };

            /** Set the process name to filter on. */
            void setProcess(std::string processName) { processName_ = processName; }; 

            /** Set the volume to which the filter will be applied to. */
            void setVolume(std::string volumeName) { volumeName_ = volumeName; };

        private:

            /** Messenger used to pass arguments to this class. */
            SimpleProcessFilterMessenger* messenger_{nullptr};
            
            /** 
             * The parent ID of the particle to which the filter will be 
             * applied to. 
             */
            int parentID_{-9999};

            /** 
             * The track ID of the particle to which the filter will be 
             * applied to. 
             */
            int trackID_{-9999};

            /** 
             * The PDG ID of the particle to which the filter will be 
             * applied to. 
             */
            int pdgID_{-9999};

            /** Process to filter on. */
            std::string processName_{""};

            /** The volume name of the LDMX target. */
            G4String volumeName_{""};
    };
}

#endif // BIASING_SIMPLEPROCESSFILTER_H__
