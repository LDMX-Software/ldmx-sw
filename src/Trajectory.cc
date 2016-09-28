#include "SimApplication/Trajectory.h"

// Geant4
#include "G4TrajectoryPoint.hh"

G4Allocator<Trajectory> TrajectoryAllocator;

Trajectory::Trajectory(const G4Track* aTrack)
    : genStatus(0),
      simStatus(0) {

    particleDef = aTrack->GetDefinition();
    mass = aTrack->GetDynamicParticle()->GetMass();
    trackID = aTrack->GetTrackID();
    parentID = aTrack->GetParentID();
    initialMomentum = aTrack->GetMomentum();
    energy = aTrack->GetTotalEnergy();
    globalTime = aTrack->GetGlobalTime();
    vertexPosition = aTrack->GetVertexPosition();

    trajPoints = new TrajectoryPointContainer();
    trajPoints->push_back(new G4TrajectoryPoint(aTrack->GetPosition()));
}

Trajectory::~Trajectory() {
    // Delete trajectory points and their container.
    size_t i;
    for(i = 0; i < trajPoints->size(); i++) {
        delete (*trajPoints)[i];
    }
    trajPoints->clear();
    delete trajPoints;
}

void Trajectory::AppendStep(const G4Step* aStep) {
   trajPoints->push_back(new G4TrajectoryPoint(aStep->GetPostStepPoint()->GetPosition()));
}

G4int Trajectory::GetTrackID() const {
    return trackID;
}

G4int Trajectory::GetParentID() const {
    return parentID;
}

G4String Trajectory::GetParticleName() const {
    return particleDef->GetParticleName();
}

G4double Trajectory::GetCharge() const {
    return particleDef->GetPDGCharge();
}

G4int Trajectory::GetPDGEncoding() const {
    return particleDef->GetPDGEncoding();
}

G4ThreeVector Trajectory::GetInitialMomentum () const {
    return initialMomentum;
}

int Trajectory::GetPointEntries() const {
    return trajPoints->size();
}

G4VTrajectoryPoint* Trajectory::GetPoint(G4int i) const {
    return (*trajPoints)[i];
}

void Trajectory::MergeTrajectory(G4VTrajectory* secondTrajectory) {
    if (secondTrajectory == NULL) {
        return;
    }

    Trajectory* seco = (Trajectory*)secondTrajectory;
    G4int ent = seco->GetPointEntries();
    for(int i=1; i<ent; i++) {
        trajPoints->push_back((*(seco->trajPoints))[i]);
    }
    delete (*seco->trajPoints)[0];
    seco->trajPoints->clear();
}

const G4ThreeVector& Trajectory::getEndPoint() const {
    return endPoint;
}

G4double Trajectory::getEnergy() const {
    return energy;
}

G4double Trajectory::getMass() const {
    return mass;
}

G4float Trajectory::getGlobalTime() const {
    return globalTime;
}

G4int Trajectory::getGenStatus() const {
    return genStatus;
}

G4int Trajectory::getSimStatus() const {
    return simStatus;
}

const G4ThreeVector& Trajectory::getVertexPosition() const {
    return vertexPosition;
}
