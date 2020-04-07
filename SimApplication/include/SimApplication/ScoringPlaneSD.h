/**
 * @file ScoringPlaneSD.h
 * @brief Class defining a basic sensitive detector for scoring planes.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_SCORINGPLANESD_H_
#define SIMAPPLICATION_SCORINGPLANESD_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//------------//   
//   Geant4   //
//------------//   
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"
#include "G4VSensitiveDetector.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

//----------//
//   LDMX   //
//----------//
#include "DetDescr/DetectorID.h"
#include "DetDescr/IDField.h"
#include "Framework/Event.h"
#include "SimApplication/G4TrackerHit.h"

namespace ldmx { 

    class ScoringPlaneSD : public G4VSensitiveDetector { 
    
        public: 

            /** 
             * Constructor
             *
             * @param name The name of the sensitive detector.
             * @param collectionID The name of the hits collection.
             * @param subDetID The subdetector ID.
             * @param detID The detector ID.
             */
            ScoringPlaneSD(G4String name, G4String colName, int subDetID, DetectorID* detID);

            /** Destructor */
            ~ScoringPlaneSD(); 

            /**
             * Process a step and create a hit out of it.
             *
             * @param step The current step.
             * @param history The readout history.
             */
            G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

            /**
             * Initialize the sensitive detector.
             *
             * @param hcEvent The hits collections associated with this event.
             */
            void Initialize(G4HCofThisEvent* hcEvent);

            /**
             * End of event hook.
             *
             * @param hcEvent The hits collections associated with this event.
             */
            void EndOfEvent(G4HCofThisEvent* hcEvent);

        private:

            /** Output hits collection */
            G4TrackerHitsCollection* hitsCollection_{nullptr};
            
            /** The detector ID. */
            DetectorID* detID_{nullptr};
            
            /** The subdetector ID. */
            int subDetID_{0};

            /** enable logging in this class */
            enableLogging( "ScoringPlaneSD" )

    }; // ScoringPlaneSD
} // ldmx

#endif // SIMAPPLICATION_SCORINGPLANESD_H_

