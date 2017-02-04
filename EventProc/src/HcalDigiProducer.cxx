#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/Event.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/HcalHit.h"
#include "EventProc/HcalDigiProducer.h"
#include "Framework/ParameterSet.h"

#include <iostream>

#include "Event/SimCalorimeterHit.h"

namespace ldmx {

HcalDigiProducer::HcalDigiProducer(const std::string& name, const Process& process) :
    Producer(name, process) {
    hits_ = new TClonesArray(EventConstants::HCAL_HIT.c_str());
}

void HcalDigiProducer::configure(const ParameterSet& ps) {
    random_ = new TRandom(ps.getInteger("randomSeed", 1000));
    meanNoise_ = ps.getDouble("meanNoise");
    mev_per_mip_ = ps.getDouble("mev_per_mip");
    pe_per_mip_ = ps.getDouble("pe_per_mip");
    verbose_ = bool(ps.getInteger("verbose"));
    detID_.setNumLayers(ps.getInteger("num_back_hcal_layers"),ps.getInteger("num_wrap_hcal_layers"));
}

void HcalDigiProducer::produce(Event& event) {

    std::map<int,int> hcalLayerPEs = detID_.getMap<int>();
    std::map<int, float> hcalLayerEdep = detID_.getMap<float>(); 
    std::map<int, float> hcalLayerTime = detID_.getMap<float>();

    // looper over sim hits and aggregate energy depositions for each detID
    TClonesArray* hcalHits = (TClonesArray*) event.getCollection(EventConstants::HCAL_SIM_HITS, "sim");

    int numHCalSimHits = hcalHits->GetEntries();
    for (int iHit = 0; iHit < numHCalSimHits; iHit++) {
        SimCalorimeterHit* simHit = (SimCalorimeterHit*) hcalHits->At(iHit);
        int detIDraw = simHit->getID();
        if (verbose_)
            std::cout << "detIDraw: " << detIDraw << std::endl;
        detID_.setRawValue(detIDraw);
        detID_.unpack();
        int layer = detID_.getFieldValue("layer");
        if (hcalLayerEdep.find(simHit->getID()) == hcalLayerEdep.end()) {
            std::cout << "ERROR: hcal digi maps not initialized properly" << std::endl;
            std::cout << "simHit detID: " << detID_.getRawValue() << std::endl;
            std::cout << "simHit detector: " << detID_.getFieldValue("subdet") << std::endl;
            std::cout << "simHit layer: " << detID_.getFieldValue("layer") << std::endl;
            std::cout << "simHit subdetector: " << detID_.getFieldValue("hcalsubdet") << std::endl;
            std::cout << "simHit pos: " << simHit->getPosition()[0] << " " << simHit->getPosition()[1] << " " << simHit->getPosition()[2] << std::endl;
            // first hit, initialize 
            hcalLayerEdep[detIDraw] = simHit->getEdep();
            hcalLayerTime[detIDraw] = simHit->getTime() * simHit->getEdep();
        } else {
            // not first hit, aggregate 
            hcalLayerEdep[detIDraw] += simHit->getEdep();
            hcalLayerTime[detIDraw] += simHit->getTime() * simHit->getEdep();
        }
    } // end loop over sim hits

    // loop over detID and simulate number of PEs
    int ihit = 0;
    for (std::map<int, float>::iterator it = hcalLayerEdep.begin(); it != hcalLayerEdep.end(); ++it) {
        int detIDraw = it->first;
        double depEnergy = hcalLayerEdep[detIDraw];
        hcalLayerTime[detIDraw] = hcalLayerTime[detIDraw] / hcalLayerEdep[detIDraw];
        double meanPE = depEnergy / mev_per_mip_ * pe_per_mip_;

        //        std::default_random_engine generator;
        //std::poisson_distribution<int> distribution(meanPE);

        hcalLayerPEs[detIDraw] = random_->Poisson(meanPE);
        hcalLayerPEs[detIDraw] += random_->Gaus(meanNoise_);

        if (verbose_) {
            std::cout << "detID: "  << detIDraw << std::endl;
            std::cout << "Layer: "  << detID_.getLayer() << std::endl;
            std::cout << "SubDet: " << detID_.getHcalSubDet() << std::endl;
            std::cout << "Edep: "   << hcalLayerEdep[detIDraw] << std::endl;
            std::cout << "numPEs: " << hcalLayerPEs[detIDraw] << std::endl;
            std::cout << "time: "   << hcalLayerTime[detIDraw] << std::endl;
        }        // end verbose 

        double energy = hcalLayerPEs[detIDraw] / pe_per_mip_ * mev_per_mip_; // need to add in a weighting factor eventually

        HcalHit *hit = (HcalHit*) (hits_->ConstructedAt(ihit));

        hit->setPE(hcalLayerPEs[detIDraw]);
        hit->setAmplitude(hcalLayerPEs[detIDraw]);
        hit->setEnergy(energy);
        hit->setTime(hcalLayerTime[detIDraw]);
        hit->setID(detIDraw);
        ihit++;
    } // end loop over detIDs (layers)
    // put it into the event
    event.add("hcalDigis", hits_);
}
    
}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);

