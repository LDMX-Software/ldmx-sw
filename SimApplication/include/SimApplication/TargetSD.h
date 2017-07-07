/**
 * @file TargetSD.h
 * @brief Class providing a sensitive detector for target or trigger pad
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_TARGETSD_H_
#define SIMAPPLICATION_TARGETSD_H_

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "Event/Event.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "DetDescr/DefaultDetectorID.h"

namespace ldmx {

/**
 * @class TargetSD
 * @brief Sensitive detector for target or trigger pad 
 * @note Same as CalorimeterSD except no layer number is set on the ID.
 */
class TargetSD: public G4VSensitiveDetector {

    public:

        /**
         * Class constructor.
         * @param name The name of the calorimeter.
         * @param theCollectionName The name of the hits collection.
         * @param detID The detector ID.
         */
        TargetSD(G4String name,
                G4String theCollectionName,
                DetectorID* detID = new DefaultDetectorID);

        /**
         * Class destructor.
         */
        virtual ~TargetSD();

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

    protected:

        /**
         * The hits collections of the sensitive detector.
         */
        G4CalorimeterHitsCollection* hitsCollection_;

        /**
         * The detector ID.
         */
        DetectorID* detID_;
};

}

#endif
