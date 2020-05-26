#include "Biasing/PhotoNuclearProductsFilter.h" 

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <cmath> 

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4Track.hh" 

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"

namespace ldmx { 

    PhotoNuclearProductsFilter::PhotoNuclearProductsFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) {
        productsPdgID_ = parameters.getParameter< std::vector< int >  >("pdg_ids");
     }

    PhotoNuclearProductsFilter::~PhotoNuclearProductsFilter() { 
    }

    void PhotoNuclearProductsFilter::stepping(const G4Step* step) { 
        
        // Get the track associated with this step.
        auto track{step->GetTrack()};

        // Get the track info and check if this track is the photon that 
        // underwent a photo-nuclear reaction.
        auto trackInfo{static_cast< UserTrackInformation* >(track->GetUserInformation())};
        if ((trackInfo != nullptr) && !trackInfo->isPNGamma()) return;

        // Get the particles daughters.
        auto secondaries{step->GetSecondary()};

        // Loop through all of the secondaries and check for the product of
        // interest
        bool productFound{false}; 
        for (const auto& secondary : *secondaries) { 
            
            // Get the PDG ID of the track
            auto pdgID{std::abs(secondary->GetParticleDefinition()->GetPDGEncoding())};
            
            // Check if the PDG ID is in the list of products of interest
            if (std::find(productsPdgID_.begin(), productsPdgID_.end(), pdgID) != productsPdgID_.end()) {
                productFound = true; 
                break; 
            }
        }

        // If the product of interest was not found, kill the track and abort 
        // the event. 
        if (!productFound) { 
            track->SetTrackStatus(fKillTrackAndSecondaries);
            G4RunManager::GetRunManager()->AbortEvent();
            return;
        }

        std::cout << "Kaon event found." << std::endl;
    }
}

DECLARE_ACTION(ldmx, PhotoNuclearProductsFilter)
