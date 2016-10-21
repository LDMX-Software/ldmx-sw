#ifndef SimApplication_Trajectory_h
#define SimApplication_Trajectory_h

// Geant4
#include "G4VTrajectory.hh"
#include "G4Allocator.hh"
#include "G4Track.hh"

// STL
#include <vector>

typedef std::vector<G4VTrajectoryPoint*> TrajectoryPointContainer;

namespace sim {

/**
 * REFERENCE
 *
 * <a href="http://geant4.slac.stanford.edu/Tips/event/3.html">Trajectory Event Tip</a>
 */
class Trajectory : public G4VTrajectory {

    public:

        Trajectory(const G4Track* aTrack);

        virtual ~Trajectory();

        inline void* operator new(size_t);

        inline void  operator delete(void*);

        G4int GetTrackID() const;

        G4int GetParentID() const;

        G4String GetParticleName() const;

        G4double GetCharge() const;

        G4int GetPDGEncoding() const;

        G4ThreeVector GetInitialMomentum () const;

        int GetPointEntries() const;

        G4VTrajectoryPoint* GetPoint(G4int) const;

        void AppendStep(const G4Step *aStep);

        void MergeTrajectory (G4VTrajectory *secondTrajectory);

        const G4ThreeVector& getEndPoint() const;

        G4double getEnergy() const;

        G4double getMass() const;

        G4float getGlobalTime() const;

        G4int getGenStatus() const;

        const G4ThreeVector& getVertexPosition() const;

        void setGenStatus(int);

    private:

        TrajectoryPointContainer* trajPoints;
        G4ParticleDefinition* particleDef;
        G4int trackID;
        G4int parentID;
        G4double energy;
        G4double mass;
        G4float globalTime;
        G4int genStatus;
        G4int simStatus;
        G4ThreeVector initialMomentum;
        G4ThreeVector vertexPosition;
        G4ThreeVector endPoint;
};

extern G4Allocator<Trajectory> TrajectoryAllocator;

inline void* Trajectory::operator new(size_t) {
    void* aTrajectory;
    aTrajectory = (void*) TrajectoryAllocator.MallocSingle();
    return aTrajectory;
}

inline void Trajectory::operator delete(void* aTrajectory) {
    TrajectoryAllocator.FreeSingle((Trajectory*) aTrajectory);
}

}

#endif

