/**
 * @file TrackProcessFilter.cxx
 * @brief Filter used to flag tracks for saving based on the process they 
 *        were created from.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TrackProcessFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Track.hh"
#include "G4VProcess.hh" 

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"

namespace ldmx { 

    TrackProcessFilter::TrackProcessFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) {

            auto processes{parameters.getParameter< std::vector< std::string > >("process")};
            for (auto& process : processes) process_.push_back(process);  
    }

    TrackProcessFilter::~TrackProcessFilter() {}

    void TrackProcessFilter::PostUserTrackingAction(const G4Track* track) {
    
        if(const G4VProcess* process{track->GetCreatorProcess()}; process) {
            auto name{process->GetProcessName()};
            for(auto& proc : process_) { 
                auto trackInfo{dynamic_cast<UserTrackInformation*>(track->GetUserInformation())};
                if (name.contains(proc)) {
                    trackInfo->setSaveFlag(true);
                } else trackInfo->setSaveFlag(false); 
            }
        } 
    }
}

DECLARE_ACTION(ldmx, TrackProcessFilter)

