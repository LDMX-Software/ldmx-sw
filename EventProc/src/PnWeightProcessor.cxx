/**
 * @file pnWeightProcessor.cxx
 * @brief Processor that calculates pnWeight based on photonNuclear track properties.
 * @author Alex Patterson, UCSB
 */

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "EventProc/PnWeightProcessor.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/DefaultDetectorID.h"

#include <iostream>


namespace ldmx {

    void PnWeightProcessor::configure(const ParameterSet& pSet) {
        wpThreshold_ = pSet.getDouble("wp_threshold");
    }

    void PnWeightProcessor::produce(Event& event) {
        result_.Clear();

        double weight = 1.;
        double test = 2.;

        const TClonesArray *simParticles = event.getCollection("SimParticles");
        if (simParticles->GetEntriesFast() == 0) return; 


        // skim simParticles for PN secondaries in Ecal
        /* TODO:
        *   in G4HadronicProcess.hh, 121 = G4HadronicProcessType.fHadronInelastic. switch to >> simParticle.GetProcessName().compareTo("photonNuclear") <<
        *   fetch ecal dimensions from somewhere
        */
        std::vector<SimParticle*> skimEcalPN;
        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); ++particleCount) {
            SimParticle* simParticle = static_cast<SimParticle*>(simParticles->At(particleCount));

            // PN secondary originating in Ecal?
            std::vector<double> vtx = simParticle->getVertex();
            double vtx_z = vtx[2];
            if(vtx_z < 200. || vtx_z > 526.) continue;
            if(simParticle->getProcessType() != 121) continue;

            skimEcalPN.push_back(simParticle);
        }

        // second skim for hardest backwards-going nucleon
        //

        std::cout << std::endl << std::endl << "[ pnWeightProcessor ] : survived skim: " << std::endl;
        for(auto skimTrack : skimEcalPN){
            printTrack(skimTrack);
        }

        // calculate PN weight
     /*
     *   fit variable W_p = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p))
     *     where p_tot = sqrt(K^2 + 2*K*m)
     *           K = kinetic energy of nucleon at PN vertex
     *           p, p_z = momentum, z-component of nucleon at PN vertex
     */


        result_.setWeight(weight);
        result_.setTest(test);

        //verbose_ = true;
        if(verbose_) std::cout << "[ pnWeightProcessor ] : pnWeight: " << result_.getWeight() << std::endl;
        if(verbose_) std::cout << "[ pnWeightProcessor ] : test: " << result_.getTest() << std::endl;
        if(verbose_) std::cout << "[ pnWeightProcessor ] : wpThreshold_: " << wpThreshold_ << std::endl;

        //Put it into the event
        event.addToCollection("pnWeight", result_);
    }

    void PnWeightProcessor::printTrack(SimParticle * inTrack){
          std::cout <<
                std::endl << "energy:      " << inTrack->getEnergy() << ", " <<
                std::endl << "PDG ID:      " << inTrack->getPdgID() << ", " <<
                std::endl << "time:        " << inTrack->getTime() << ", " <<
                std::endl << "vertex:    ( " << inTrack->getVertex()[0] << ", " << inTrack->getVertex()[1] << ", " << inTrack->getVertex()[2] << " ), " <<
                std::endl << "endPoint:  ( " << inTrack->getEndPoint()[0] << ", " << inTrack->getEndPoint()[1]  << ", " << inTrack->getEndPoint()[2]  << " ), " <<
                std::endl << "momentum:  ( " << inTrack->getMomentum()[0] << ", " << inTrack->getMomentum()[1] << ", " << inTrack->getMomentum()[2] << " ), " <<
                std::endl << "endPointMomentum: ( " << inTrack->getEndPointMomentum()[0] << ", " << inTrack->getEndPointMomentum()[1] << ", " << inTrack->getEndPointMomentum()[2] << " ), " <<
                std::endl << "mass:        " << inTrack->getMass() << ", " <<
                std::endl << "nDaughters:  " << inTrack->getDaughterCount() << ", " <<
                std::endl << "nParents:    " << inTrack->getParentCount() << ", " <<
                std::endl << "processType: " << inTrack->getProcessType() <<
                std::endl << std::endl;
    }

}

DECLARE_PRODUCER_NS(ldmx, PnWeightProcessor)
