/**
 * @file RootPrimaryGenerator.h
 * @brief Primary generator used to generate primaries from SimParticles. 
 * @author Nhan Tran, Fermilab
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_ROOTPRIMARYGENERATOR_H_
#define SIMAPPLICATION_ROOTPRIMARYGENERATOR_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <fstream>
#include <iostream>

//------------//
//   Geant4   //
//------------//
#include "G4VPrimaryGenerator.hh"

//----------//
//   ROOT   //
//----------//
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TVector3.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/EventHeader.h"

namespace ldmx {

    class RootPrimaryGenerator : public G4VPrimaryGenerator {

        public:

            /**
             * Class constructor.
             * @param reader The LHE reader with the event data.
             */
            RootPrimaryGenerator(G4String filename);

            /**
             * Class destructor.
             */
            virtual ~RootPrimaryGenerator();

            /**
             * Specify the run mode
             * @param the mode index.
             */
            void setRunMode(int curmode) { runMode_ = curmode; };

            /**
             * Generate vertices in the Geant4 event.
             * @param anEvent The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* anEvent);

        private:

            /**
             * The root filename
             */
            G4String filename_;

            /**
             * The root file
             */
            TFile* ifile_;

            /**
             * The ldmx root tree
             */
            TTree* itree_;

            /**
             * The sim particles
             */
            TClonesArray* simParticles_;
            TClonesArray* ecalSPParticles_;

            /**
             * The event header
             */
            ldmx::EventHeader* eventHeader_;

            /**
             * The event counter
             */
            int evtCtr_;

            /**
             * Events in the tree...
             */
            int nEvts_;

            /**
             * Run mode...
             */
            int runMode_;

    };

}

#endif
