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

#include "DetDescr/DefaultDetectorID.h"
#include "Event/SimCalorimeterHit.h"

namespace ldmx {

    //const float HcalDigiProducer::MEV_PER_MIP = 1.40;
    //const float HcalDigiProducer::PE_PER_MIP = 13.5 * 6. / 4.;

HcalDigiProducer::HcalDigiProducer(const std::string& name, const Process& process) :
        Producer(name, process) {
    hits_ = new TClonesArray(EventConstants::HCAL_HIT.c_str());
}

void HcalDigiProducer::configure(const ParameterSet& ps) {
    detID_ = new DefaultDetectorID();
    random_ = new TRandom(ps.getInteger("randomSeed", 1000));
    meanNoise_ = ps.getDouble("meanNoise");
    mev_per_mip_ = ps.getDouble("mev_per_mip");
    pe_per_mip_ = ps.getDouble("pe_per_mip");
    
}

void HcalDigiProducer::produce(Event& event) {

    std::map<int, int> hcalLayerNum, hcalLayerPEs, hcalDetId;
    std::map<int, float> hcalZpos;
    std::map<int, float> hcalLayerEdep, hcalLayerTime;

    // looper over sim hits and aggregate energy depositions for each detID
    TClonesArray* hcalHits = (TClonesArray*) event.getCollection(EventConstants::HCAL_SIM_HITS, "sim");

    int numHCalSimHits = hcalHits->GetEntries();
    for (int iHit = 0; iHit < numHCalSimHits; iHit++) {
        SimCalorimeterHit* simHit = (SimCalorimeterHit*) hcalHits->At(iHit);
        //int detID=simHit->getID();
        int detIDraw = simHit->getID();
        if (verbose_)
            std::cout << "detIDraw: " << detIDraw << std::endl;
        detID_->setRawValue(detIDraw);
        detID_->unpack();
        int layer = detID_->getFieldValue("layer");
        if (verbose_)
            std::cout << "layer: " << layer << std::endl;
        if (hcalLayerEdep.find(simHit->getID()) == hcalLayerEdep.end()) {
            // first hit, initialize 
            hcalLayerEdep[detIDraw] = simHit->getEdep();
            hcalLayerTime[detIDraw] = simHit->getTime() * simHit->getEdep();
            hcalDetId[detIDraw] = detIDraw;
            hcalZpos[detIDraw] = simHit->getPosition()[2];
            hcalLayerNum[detIDraw] = layer;
        } else {
            // not first hit, aggregate 
            hcalLayerEdep[detIDraw] += simHit->getEdep();
            hcalLayerTime[detIDraw] += simHit->getTime() * simHit->getEdep();
        }
    } // end loop over sim hits

    // loop over detID (layers) and simulate number of PEs
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
            std::cout << "detID: " << detIDraw << std::endl;
            std::cout << "Layer: " << hcalLayerNum[detIDraw] << std::endl;
            std::cout << "Edep: " << hcalLayerEdep[detIDraw] << std::endl;
            std::cout << "numPEs: " << hcalLayerPEs[detIDraw] << std::endl;
            std::cout << "time: " << hcalLayerTime[detIDraw] << std::endl;
            std::cout << "z: " << hcalZpos[detIDraw] << std::endl;
        }        // end verbose 

        int layer = hcalLayerNum[detIDraw];
        double energy = hcalLayerPEs[detIDraw] / pe_per_mip_ * mev_per_mip_; // need to add in a weighting factor eventually

        HcalHit *hit = (HcalHit*) (hits_->ConstructedAt(ihit));

        //	hit->setLayer(layer);
        hit->setPE(hcalLayerPEs[detIDraw]);
        hit->setAmplitude(hcalLayerPEs[detIDraw]);
        hit->setEnergy(energy);
        hit->setTime(hcalLayerTime[detIDraw]);
        hit->setID(detIDraw);
        //	hit->setZpos(hcalZpos[detIDraw]);
        ihit++;
    } // end loop over detIDs (layers)
      // put it into the event
    event.add("hcalDigis", hits_);
}

}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);

