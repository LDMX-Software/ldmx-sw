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
//#include "Event/PnWeightResult.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
//#include "Event/HcalHit.h"
//#include "EventProc/HcalDigiProducer.h"
#include "DetDescr/DefaultDetectorID.h"

#include <iostream>


namespace ldmx {


    //const float HcalDigiProducer::PE_PER_MIP = 13.5 * 6. / 4.;

/*
    PnWeightProcessor::PnWeightProcessor(const std::string& name, Process& process) :
        Producer(name, process) {
    }

    PnWeightProcessor::PnWeightProcessor() {
    }
*/

    void PnWeightProcessor::configure(const ParameterSet& pSet) {
        wpThreshold_ = pSet.getDouble("wp_threshold");
        //result_ = new TClonesArray(EventConstants::PN_WEIGHT.c_str());
    }

    void PnWeightProcessor::produce(Event& event) {

        const TClonesArray *simParticles = event.getCollection("SimParticles");
        if (simParticles->GetEntriesFast() == 0) return; 

        double weight = 1.;

        // Get PN secondaries in the Ecal
        int resultCount = 0;
        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); ++particleCount) {

            SimParticle* simParticle = static_cast<SimParticle*>(simParticles->At(particleCount));
            //if (abs(simParticle->getCharge()) != 1) continue;

            /*

            */

            resultCount++;
        }

        result_.setWeight(weight);

        verbose_ = true;
        if(verbose_) std::cout << "[ pnWeightProcessor ] : pnWeight: " << result_.getWeight() << std::endl;
        if(verbose_) std::cout << "[ pnWeightProcessor ] : wpThreshold_: " << wpThreshold_ << std::endl;

        //Put it into the event
        event.addToCollection("pnWeight", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, PnWeightProcessor)
