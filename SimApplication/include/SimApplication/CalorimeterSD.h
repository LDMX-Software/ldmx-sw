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
#include "Framework/Event.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "DetDescr/DetectorID.h"

using ldmx::DetectorID;

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
             * @param subdet The subdetector ID.
             * @param detID The detector ID.
             */
            CalorimeterSD(G4String name, G4String theCollectionName, int subdet, DetectorID* detID);

            /**
             * Class destructor.
             */
            virtual ~CalorimeterSD();

            /**
             * Set the depth into the geometric hierarchy to the layer volume.
             * @param layerDepth The depth into the geometric hierarchy to the layer volume.
             */
            void setLayerDepth(int layerDepth) {
                this->layerDepth_ = layerDepth;
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

        protected:

            /**
             * The hits collections of the sensitive detector.
             */
            G4CalorimeterHitsCollection* hitsCollection_;

            /**
             * The subdetector ID.
             */
            int subdet_;

            /**
             * The detector ID.
             */
            DetectorID* detID_;

            /**
             * The depth to the layer volume.
             */
            int layerDepth_ {2};

            /**
             * Allow for logging in this class
             */
            enableLogging( "CalorimeterSD" )
    };

}

#endif
