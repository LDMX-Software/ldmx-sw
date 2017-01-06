/**
 * @file TrackerSD.h
 * @brief Class defining a basic sensitive detector for trackers
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_TRACKERSD_H_
#define SIMAPPLICATION_TRACKERSD_H_

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "SimApplication/G4TrackerHit.h"
#include "DetDescr/DetectorID.h"
#include "Event/Event.h"

using event::Event;
using detdescr::DetectorID;

namespace sim {

/**
 * @class TrackerSD
 * @brief Basic sensitive detector for trackers
 *
 * @note
 * This class creates a G4TrackerHit for each step within the subdetector.
 */
class TrackerSD: public G4VSensitiveDetector {

    public:

        /**
         * Class constructor.
         * @param name The name of the sensitive detector.
         * @param theCollectionName The name of the hits collection.
         * @param subdetID The subdetector ID.
         * @param detID The detector ID.
         */
        TrackerSD(G4String name,
                G4String theCollectionName,
                int subdetID,
                DetectorID* detID);

        /**
         * Class destructor.
         */
        virtual ~TrackerSD();

        /**
         * Set the detector ID.
         * @param detID The detector ID.
         */
        void setDetectorID(DetectorID* detID) {
            this->detID_= detID;
        }

        /**
         * Process a step by creating a hit.
         * @param aStep The step information
         * @param ROhist The readout history.
         */
        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

        /**
         * Initialize the sensitive detector.
         * @param hcEvent The hits collections of the event.
         */
        void Initialize(G4HCofThisEvent* hcEvent);

        /**
         * End of event hook.
         * @param hcEvent The hits collections of the event.
         */
        void EndOfEvent(G4HCofThisEvent* hcEvent);

    private:

        /**
         * The output hits collection of G4TrackerHits.
         */
        G4TrackerHitsCollection* hitsCollection_;

        /**
         * The subdetector ID.
         */
        int subdetID_;

        /**
         * The detector ID.
         */
        DetectorID* detID_;
};

}

#endif
