#ifndef SIMAPPLICATION_TRACKSUMMARY_H_
#define SIMAPPLICATION_TRACKSUMMARY_H_ 1

// Geant4
#include "globals.hh"
#include "G4Allocator.hh"
#include "G4Track.hh"
#include "G4ThreeVector.hh"

// TODO: This class could extend G4VTrajectory.
class TrackSummary {

    public:

        typedef std::map<G4int, TrackSummary*> TrackSummaryMap;
        typedef std::vector<TrackSummary*> TrackSummaryList;

        TrackSummary(const G4Track* aTrack);

        virtual ~TrackSummary();

        void update(const G4Track* track);

        // G4VTrajectory::GetCharge()
        G4double getCharge() const;

        const G4ThreeVector& getEndPoint() const;

        G4double getEnergy() const;

        G4double getMass() const;

        G4float getGlobalTime() const;

        G4int getGenStatus() const;

        G4int getSimStatus() const;

        // G4VTrajectory::GetInitialMomentum()
        const G4ThreeVector& getMomentum() const;

        // G4VTrajectory::GetPDGEncoding()
        G4int getPDG() const;

        const G4ThreeVector& getVertex() const;

        // G4VTrajectory::GetTrackID()
        G4int getTrackID() const;

        // G4VTrajectory::GetParentID()
        G4int getParentID() const;

        G4double getTrackLength() const;

        G4bool getSaveFlag() const;

        void setSaveFlag(bool aSaveFlag);

        TrackSummary* findParent();

        TrackSummary* findFirstSavedParent();

        static TrackSummaryMap* getTrackSummaryMap();

        static TrackSummaryList* getTrackSummaryList();

        static void clearRegistry();

    private:

        G4double charge;
        G4ThreeVector endPoint;
        G4double energy;
        G4double mass;
        G4float globalTime;
        G4int genStatus;
        G4int simStatus;
        G4ThreeVector momentum;
        G4int pdgID;
        G4ThreeVector vertex;
        G4int trackID;
        G4int parentID;
        G4bool saveFlag;
        TrackSummary* parent;

        static TrackSummaryMap trackMap;
        static TrackSummaryList trackList;
};

#endif

