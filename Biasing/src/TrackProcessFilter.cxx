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

        process_ = parameters.getParameter< std::string >("process");
    }

    TrackProcessFilter::~TrackProcessFilter() {}

    void TrackProcessFilter::PostUserTrackingAction(const G4Track* track) {
    
        if(const G4VProcess* process{track->GetCreatorProcess()}; process) {
            auto name{process->GetProcessName()};
            auto trackInfo{dynamic_cast<UserTrackInformation*>(track->GetUserInformation())};
            if (name.contains(process_)) {
                trackInfo->setSaveFlag(true);
            } else trackInfo->setSaveFlag(false); 
        } 
    }
}

DECLARE_ACTION(ldmx, TrackProcessFilter)

