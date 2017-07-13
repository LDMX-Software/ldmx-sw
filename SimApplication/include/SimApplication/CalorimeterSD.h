/**
 * @file CalorimeterSD.h
 * @brief Class providing a basic calorimeter sensitive detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_CALORIMETERSD_H_
#define SIMAPPLICATION_CALORIMETERSD_H_

// Geant4
#include "G4VSensitiveDetector.hh"

// LDMX
#include "Event/Event.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "DetDescr/DefaultDetectorID.h"

namespace ldmx {

    /**
     * @class CalorimeterSD
     * @brief Basic calorimeter sensitive detector
     */
    class CalorimeterSD : public G4VSensitiveDetector {

        public:

            /**
             * Class constructor.
             * @param name The name of the calorimeter.
             * @param theCollectionName The name of the hits collection.
             * @param detID The detector ID.
             */
            CalorimeterSD(G4String name, G4String theCollectionName, DetectorID* detID = new DefaultDetectorID);

            /**
             * Class destructor.
             */
            virtual ~CalorimeterSD();

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
