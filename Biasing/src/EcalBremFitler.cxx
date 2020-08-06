
#include "Biasing/EcalBremFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"
#include "G4EventManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"
#include "SimCore/UserEventInformation.h" 

namespace ldmx { 

    EcalBremFilter::EcalBremFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) { 
        
        bremEnergyThreshold_     = parameters.getParameter< double >("brem_min_energy_threshold"); 
    }

    EcalBremFilter::~EcalBremFilter() {
    }

    G4ClassificationOfNewTrack EcalBremFilter::ClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;

        if (track->GetTrackID() == 1 && pdgID == 11) {
            return fWaiting; 
        }

        return classification;
    }

    void EcalBremFilter::stepping(const G4Step* step) { 

        // Get the track associated with this step.
        auto track{step->GetTrack()};

        // Only process the primary electron track
        if (track->GetParentID() != 0) return;

        // Get the PDG ID of the track and make sure it's an electron. If 
        // another particle type is found, thrown an exception. 
        if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11) return; 

        // Get the region the particle is currently in.  Continue processing
        // the particle only if it's in the CalorimeterRegion region. 
        if (auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
                region.compareTo("CalorimeterRegion") != 0) return;

        if ( track->GetKineticEnergy() < bremEnergyThreshold_ ) {
            //are we below the energy we want?
            // Get the electron secondries
            bool hasBremCandidate = false; 
            auto secondaries = step->GetSecondary();

            std::cout << "[ EcalBremFilter ] : Primary electron went below brem energy threshold: " 
                << "KE: " << track->GetKineticEnergy() << "MeV "
                << "N Secondaries: " << secondaries->size() << std::endl;
            for (auto& secondary_track : *secondaries) {
                G4String processName = secondary_track->GetCreatorProcess()->GetProcessName();
                
                if (processName.compareTo("eBrem") == 0 
                        && secondary_track->GetKineticEnergy() > bremEnergyThreshold_) {
    
                    std::cout << "[ EcalBremFilter ] : Found a secondary hard brem!" << std::endl;
                   
                    if (secondary_track->GetUserInformation() == nullptr) {
                        secondary_track->SetUserInformation(new UserTrackInformation()); 
                    }
                    auto trackInfo{static_cast< UserTrackInformation* >(secondary_track->GetUserInformation())};
                    trackInfo->tagBremCandidate(); 
                    trackInfo->setVertexVolume(secondary_track->GetVolume()->GetName()); 
    
                    auto event{G4EventManager::GetEventManager()}; 
                    if (event->GetUserInformation() == nullptr) { 
                        event->SetUserInformation(new UserEventInformation()); 
                    }
                    static_cast< UserEventInformation* >(event->GetUserInformation())->incBremCandidateCount(); 
    
                    hasBremCandidate = true;
                } 
            }
    
            if (!hasBremCandidate) { 
                std::cout << "[ EcalBremFilter ] : No hard-brem secondaries. Aborting event..." << std::endl;
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
        }// electron below brem energy threshold
    }
}

DECLARE_ACTION(ldmx, EcalBremFilter) 
