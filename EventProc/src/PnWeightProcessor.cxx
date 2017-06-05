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
        for(auto thing : skimEcalPN){
          std::cout <<
                "energy: " << thing->getEnergy() << ", " <<
                std::endl << "PDG ID: " << thing->getPdgID() << ", " <<
                std::endl << "time: " << thing->getTime() << ", " <<
                std::endl << "vertex: ( " << thing->getVertex()[0] << ", " << thing->getVertex()[1] << ", " << thing->getVertex()[2] << " ), " <<
                std::endl << "endPoint: ( " << thing->getEndPoint()[0] << ", " << thing->getEndPoint()[1]  << ", " << thing->getEndPoint()[2]  << " ), " <<
                std::endl << "momentum: ( " << thing->getMomentum()[0] << ", " << thing->getMomentum()[1] << ", " << thing->getMomentum()[2] << " ), " <<
                std::endl << "endPointMomentum: ( " << thing->getEndPointMomentum()[0] << ", " << thing->getEndPointMomentum()[1] << ", " << thing->getEndPointMomentum()[2] << " ), " <<
                std::endl << "mass: " << thing->getMass() << ", " <<
                std::endl << "nDaughters: " << thing->getDaughterCount() << ", " <<
                std::endl << "nParents: " << thing->getParentCount() << ", " <<
                std::endl << "processType: " << thing->getProcessType() <<
                std::endl << std::endl;

//          thing->Print();
//            TString::Format("ptr %x, pdgid %d, start z %f, parents %d, daughters %d", thing, 
//            thing->getPdgID(), thing->getVertex()[2], thing->getParentCount(), thing->getDaughterCount()) << std::endl;
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

    void PrintSimParticle(const SimParticle * 

}

DECLARE_PRODUCER_NS(ldmx, PnWeightProcessor)
