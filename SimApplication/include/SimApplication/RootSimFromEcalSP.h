/**
 * @file RootPrimaryGenerator.h
 * @brief Primary generator used to generate primaries from SimParticles. 
 * @author Nhan Tran, Fermilab
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMAPPLICATION_ROOTPRIMARYGENERATOR_H
#define SIMAPPLICATION_ROOTPRIMARYGENERATOR_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <fstream>
#include <iostream>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/EventDef.h"
#include "SimApplication/PrimaryGenerator.h"

class G4Event;

namespace ldmx {

    class Parameters;

    class RootPrimaryGenerator : public PrimaryGenerator {

        public:

            /**
             * Class constructor.
             * @param reader The LHE reader with the event data.
             */
            RootPrimaryGenerator(const std::string& name, Parameters& parameters);

            /**
             * Class destructor.
             */
            virtual ~RootPrimaryGenerator();

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
            std::vector<SimParticle> simParticles_;
            std::vector<SimTrackerHit> ecalSPParticles_;

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

#endif //SIMAPPLICATION_ROOTPRIMARYGENERATOR_H
