#include "SimApplication/Trajectory.h"

// Geant4
#include "G4TrajectoryPoint.hh"

namespace sim {

G4Allocator<Trajectory> TrajectoryAllocator;

Trajectory::Trajectory(const G4Track* aTrack)
    : genStatus_(0),
      simStatus_(0) {

    particleDef_ = aTrack->GetDefinition();
    mass_ = aTrack->GetDynamicParticle()->GetMass();
    trackID_ = aTrack->GetTrackID();
    parentID_ = aTrack->GetParentID();
    initialMomentum_ = aTrack->GetMomentum();
    energy_ = aTrack->GetTotalEnergy();
    globalTime_ = aTrack->GetGlobalTime();
    vertexPosition_ = aTrack->GetVertexPosition();

    trajPoints_ = new TrajectoryPointContainer();
    trajPoints_->push_back(new G4TrajectoryPoint(aTrack->GetPosition()));
}

Trajectory::~Trajectory() {
    // Delete trajectory points and their container.
    size_t i;
    for(i = 0; i < trajPoints_->size(); i++) {
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

G4ThreeVector Trajectory::GetInitialMomentum () const {
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

    Trajectory* seco = (Trajectory*)secondTrajectory;
    G4int ent = seco->GetPointEntries();
    for(int i=1; i<ent; i++) {
        trajPoints_->push_back((*(seco->trajPoints_))[i]);
    }
    delete (*seco->trajPoints_)[0];
    seco->trajPoints_->clear();
}

const G4ThreeVector& Trajectory::getEndPoint() const {
    return endPoint_;
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

}
