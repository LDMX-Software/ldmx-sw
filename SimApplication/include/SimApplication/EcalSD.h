/**
 * @file EcalSD.h
 * @brief Class defining an ECal sensitive detector using an EcalHexReadout to create the hits
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_ECALSD_H_
#define SIMAPPLICATION_ECALSD_H_

// LDMX
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "SimApplication/CalorimeterSD.h"

// ROOT
#include "TMath.h"

// Geant4
#include "G4Polyhedra.hh"

namespace ldmx {

    /**
     * @class EcalSD
     * @brief ECal sensitive detector that uses an EcalHexReadout to create the hits
     */
    class EcalSD : public CalorimeterSD {

        public:

            /**
             * Class constructor.
             * @param name The name of the sensitive detector.
             * @param theCollectionName The name of the hits collection.
             * @param subDetID The subdetector ID.
             */
            EcalSD(G4String name, G4String theCollectionName, int subDetID);

            /**
             * Class destructor.
             */
            virtual ~EcalSD();

            /**
             * Process steps to create hits.
             * @param aStep The step information.
             * @param ROhist The readout history.
             */
            G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

        private:

            /**
             * Return the hit position of a step.
             * X and Y are computed from the midpoint of the step.
             * Z corresponds to the volume's center.
             * @return The hit position from the step.
             * @todo This function is probably slow due to it inspecting the
             * geometry to get the Z position so this should be sped up somehow.
             */
            G4ThreeVector getHitPosition(G4Step* aStep);

        private:

            /**
             * The hex readout defining the cell grid.
             */
            std::unique_ptr<EcalHexReadout> hitMap_;

            /**
             * Map of polygonal layers for getting Z positions.
             */
            std::map<G4VSolid*, G4Polyhedron*> polyMap_;
    };

}

#endif
