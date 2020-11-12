#include "SimCore/Trajectory.h"

// LDMX
#include "SimCore/UserTrackInformation.h"
#include "SimCore/Event/SimParticle.h"

// Geant4
#include "G4TrajectoryPoint.hh"
#include "G4VProcess.hh"

namespace ldmx {

    G4Allocator<Trajectory> TrajectoryAllocator;

    Trajectory::Trajectory(const G4Track* aTrack) :
            genStatus_(0) {

        // Copy basic info from the track.
        particleDef_ = aTrack->GetDefinition();
        mass_ = aTrack->GetDynamicParticle()->GetMass();
        trackID_ = aTrack->GetTrackID();
        parentID_ = aTrack->GetParentID();
        globalTime_ = aTrack->GetGlobalTime();
        vertexPosition_ = aTrack->GetVertexPosition();
        energy_ = aTrack->GetVertexKineticEnergy() + mass_;

        // Set the process type.
        const G4VProcess* process = aTrack->GetCreatorProcess();
        if (process) {
            const G4String& processName = process->GetProcessName();
            SimParticle::ProcessType processType = SimParticle::findProcessType(processName);
            processType_ = processType;
            // Uncomment this to see what process types are being saved.  --JM
            //std::cout << "Trajectory - set process type " << processType
            //        << " from <" << processName << ">" << std::endl;
        } else {
            processType_ = SimParticle::ProcessType::unknown;
        }

        // Get the track information. This is used to set the track vertex 
        // volume and initial momentum. 
        auto trackInfo{dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation())};
       
        // Set the vertex volume
        vertexVolume_ = trackInfo->getVertexVolume(); 
        vertexRegion_ = aTrack->GetLogicalVolumeAtVertex()->GetRegion()->GetName();

        // Set initial momentum from track information.
        const G4ThreeVector& p = trackInfo->getInitialMomentum();
        initialMomentum_.set(p.x(), p.y(), p.z());

        // If the track has not been stepped, then only the first point is added.
        // Otherwise, the track has already been stepped so we add also its last location
        // which should be its endpoint.
        trajPoints_ = new TrajectoryPointContainer();
        trajPoints_->push_back(new G4TrajectoryPoint(aTrack->GetVertexPosition()));
        if (aTrack->GetTrackStatus() == G4TrackStatus::fStopAndKill) {
            trajPoints_->push_back(new G4TrajectoryPoint(aTrack->GetPosition()));
        }
    }

    Trajectory::~Trajectory() {
        // Delete trajectory points and their container.
        size_t i;
        for (i = 0; i < trajPoints_->size(); i++) {
            delete (*trajPoints_)[i];
        }
        trajPoints_->clear();
        delete trajPoints_;
    }

    void Trajectory::AppendStep(const G4Step* aStep) {
        trajPoints_->push_back(new G4TrajectoryPoint(aStep->GetPostStepPoint()->GetPosition()));
    }

    G4int Trajectory::GetTrackID() const {
        return trackID_;
    }

    G4int Trajectory::GetParentID() const {
        return parentID_;
    }

    G4String Trajectory::GetParticleName() const {
        return particleDef_->GetParticleName();
    }

    G4double Trajectory::GetCharge() const {
        return particleDef_->GetPDGCharge();
    }

    G4int Trajectory::GetPDGEncoding() const {
        return particleDef_->GetPDGEncoding();
    }

    G4ThreeVector Trajectory::GetInitialMomentum() const {
        return initialMomentum_;
    }

    int Trajectory::GetPointEntries() const {
        return trajPoints_->size();
    }

    G4VTrajectoryPoint* Trajectory::GetPoint(G4int i) const {
        return (*trajPoints_)[i];
    }

    void Trajectory::MergeTrajectory(G4VTrajectory* secondTrajectory) {
        if (secondTrajectory == NULL) {
            return;
        }

        Trajectory* seco = (Trajectory*) secondTrajectory;
        G4int ent = seco->GetPointEntries();
        for (int i = 1; i < ent; i++) {
            trajPoints_->push_back((*(seco->trajPoints_))[i]);
        }
        delete (*seco->trajPoints_)[0];
        seco->trajPoints_->clear();
    }

    G4ThreeVector Trajectory::getEndPoint() const {
        return GetPoint(GetPointEntries() - 1)->GetPosition();
    }

    G4double Trajectory::getEnergy() const {
        return energy_;
    }

    G4double Trajectory::getMass() const {
        return mass_;
    }

    G4float Trajectory::getGlobalTime() const {
        return globalTime_;
    }

    G4int Trajectory::getGenStatus() const {
        return genStatus_;
    }

    const G4ThreeVector& Trajectory::getVertexPosition() const {
        return vertexPosition_;
    }

    void Trajectory::setGenStatus(int theGenStatus) {
        genStatus_ = theGenStatus;
    }

    Trajectory* Trajectory::findByTrackID(G4TrajectoryContainer* trajCont, int trackID) {
        TrajectoryVector* vec = trajCont->GetVector();
        for (TrajectoryVector::const_iterator it = vec->begin(); it != vec->end(); it++) {
            if ((*it)->GetTrackID() == trackID) {
                return dynamic_cast<Trajectory*>(*it);
            }
        }
        return nullptr;
    }

}
