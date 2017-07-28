/**
 * @file SimpleProcessFilter.cxx
 * @brief Class defining a UserActionPlugin that filters out events if a
 *        particle doesn't interact via a specified process.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/SimpleProcessFilter.h"

namespace ldmx { 

    extern "C" SimpleProcessFilter* createSimpleProcessFilter() {
        return new SimpleProcessFilter;
    }

    extern "C" void destroySimpleProcessFilter(SimpleProcessFilter* object) {
        delete object;
    }

    SimpleProcessFilter::SimpleProcessFilter() {
        messenger_ = new SimpleProcessFilterMessenger(this);
    }

    SimpleProcessFilter::~SimpleProcessFilter() {
    }

    void SimpleProcessFilter::stepping(const G4Step* step) { 

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // If enabled, only process tracks whose parent is equal to the given
        // parent ID.
        if ((parentID_ != -9999) && (track->GetParentID() != parentID_)) return; 

        // If enabled, only process tracks whose ID is equal to the given
        // track ID.
        if ((trackID_ != -9999) && (track->GetTrackID() != trackID_)) return;

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // if enabled, only process tracks whose PDG ID is equal to the given
        // PDG ID 
        if ((pdgID_ != -9999) && (pdgID != pdgID_)) return;

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();

        // If enabled, make sure the particle is in the specified volume. 
        // If not, strop processing the track.
        if (volumeName.compareTo(volumeName_) != 0) return;

        /*std::cout << "*******************************" << std::endl; 
        std::cout << "*   Step " << track->GetCurrentStepNumber() << std::endl;
        std::cout << "********************************" << std::endl;*/

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        // Get the kinetic energy of the particle.
        double incidentParticleEnergy = step->GetPreStepPoint()->GetTotalEnergy();

        std::cout << "[ SimpleProcessFilter ]: " << "\n" 
                    << "\tTotal energy of " << particleName      << " ( PDG ID: " << pdgID
                    << " ) : " << incidentParticleEnergy       << "\n"
                    << "\tTrack ID: " << track->GetTrackID()     << "\n" 
                    << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                    << "\tParticle currently in " << volumeName  << std::endl;
        
        // Get the particles daughters.
        const G4TrackVector* secondaries = step->GetSecondary();

        // If the brem photon doesn't undergo any reaction in the target, stop
        // processing the rest of the event. 
        if (secondaries->size() == 0) {

            std::cout << "[ SimpleProcessFilter ]: "
                      << particleName << " did not interact in " << volumeName_ 
                      << " --> Postponing tracks."
                      << std::endl;

            track->SetTrackStatus(fKillTrackAndSecondaries);
            G4RunManager::GetRunManager()->AbortEvent();
            return;
        
        } else { 
       
            G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
            
            std::cout << "[ SimpleProcessFilter ]: "
                      << particleName << " produced " << secondaries->size() 
                      << " secondaries via " << processName << " process." 
                      << std::endl;
            if (processName.find("biasWrapper") != std::string::npos) { 
                std::size_t pos = processName.find_first_of("(") + 1;
                processName = processName.substr(pos, processName.size() - pos - 1); 
            }  

            // Only record the process that is being biased
            if (!processName.empty() && (processName.compare(processName_) != 0)) {

                std::cout << "[ SimpleProcessFilter ]: "
                          << "Secondaries were not produced via " 
                          << processName_ << " --> Killing all tracks!" 
                          << std::endl;
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }

            std::cout << "[ SimpleProcessFilter ]: "
                      << particleName << " produced " << secondaries->size() 
                      << " secondaries via " << processName << " process." 
                      << std::endl;
            BiasingMessenger::setEventWeight(track->GetWeight());
        }
    }    
}

