#ifndef SIMAPPLICATION_TRACKSUMMARY_H_
#define SIMAPPLICATION_TRACKSUMMARY_H_ 1

// Geant4
#include "globals.hh"
#include "G4Allocator.hh"
#include "G4Track.hh"
#include "G4ThreeVector.hh"

class TrackSummary {

    public:

        typedef std::map<G4int, TrackSummary*> TrackSummaryMap;
        typedef std::vector<TrackSummary*> TrackSummaryList;

        TrackSummary(const G4Track* aTrack);

        virtual ~TrackSummary();

        void update(const G4Track* track);

        G4double getCharge() const;

        const G4ThreeVector& getEndPoint() const;

        G4double getEnergy() const;

        G4double getMass() const;

        G4float getGlobalTime() const;

        G4int getGenStatus() const;

        G4int getSimStatus() const;

        const G4ThreeVector& getMomentum() const;

        G4int getPDG() const;

        const G4ThreeVector& getVertex() const;

        G4int getTrackID() const;

        G4int getParentID() const;

        G4double getTrackLength() const;

        G4bool getSaveFlag() const;

        void setSaveFlag(bool aSaveFlag);

        TrackSummary* findParent();

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
        TrackSummary* parentInfo;

        static TrackSummaryMap trackMap;
        static TrackSummaryList trackList;
};

#endif

