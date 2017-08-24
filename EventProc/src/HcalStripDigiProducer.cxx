#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/HcalStripHit.h"
#include "EventProc/HcalStripDigiProducer.h"

#include <iostream>

#include "DetDescr/HcalID.h"
#include "Event/HcalStripHit.h"

namespace ldmx {

    const float HcalStripDigiProducer::FIRST_LAYER_ZPOS = 569.5;
    const float HcalStripDigiProducer::LAYER_ZWIDTH = 60.;
    const int HcalStripDigiProducer::NUM_HCAL_LAYERS = 15;
    const float HcalStripDigiProducer::MEV_PER_MIP = 1.40;
    const float HcalStripDigiProducer::PE_PER_MIP = 13.5 * 6. / 4.;

    HcalStripDigiProducer::HcalStripDigiProducer(const std::string& name, Process& process) :
            Producer(name, process) {
        hits_ = new TClonesArray(EventConstants::HCAL_STRIP_HIT.c_str());
    }

    void HcalStripDigiProducer::configure(const ParameterSet& ps) {
        detID_ = new HcalID();
        random_ = new TRandom(ps.getInteger("randomSeed", 1000));
        meanNoise_ = ps.getDouble("meanNoise");
    }

    void HcalStripDigiProducer::produce(Event& event) {

        std::map<int, int>   hcalLayerPEs, hcalDetId;
        std::map<int, float> hcalXpos,hcalYpos,hcalZpos;
        std::map<int, float> hcalLayerEdep, hcalLayerTime;

        // looper over sim hits and aggregate energy depositions for each detID
        TClonesArray* hcalHits = (TClonesArray*) event.getCollection(EventConstants::HCAL_SIM_HITS, "sim");

        int numHCalSimHits = hcalHits->GetEntries();
        for (int iHit = 0; iHit < numHCalSimHits; iHit++) {
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) hcalHits->At(iHit);
            int detIDraw = simHit->getID();
                
            if (verbose_) {
                std::cout << "detIDraw: " << detIDraw << std::endl;
                detID_->setRawValue(detIDraw);
                detID_->unpack();
                int layer = detID_->getFieldValue("layer");
                int subsection = detID_->getFieldValue("section");
                int strip = detID_->getFieldValue("strip");                
                std::cout << "section: " << subsection << "  layer: " << layer <<  "  strip: " << strip <<std::endl;
            }           

            if (hcalLayerEdep.find(simHit->getID()) == hcalLayerEdep.end()) {
                // first hit, initialize
                std::vector<float> position = simHit->getPosition();
                hcalLayerEdep[detIDraw] = simHit->getEdep();
                hcalLayerTime[detIDraw] = simHit->getTime() * simHit->getEdep();
                hcalDetId[detIDraw] = detIDraw;
                hcalXpos[detIDraw]  = position[0]* simHit->getEdep();
                hcalYpos[detIDraw]  = position[1]* simHit->getEdep();
                hcalZpos[detIDraw]  = position[2]* simHit->getEdep();
            } else {
                // not first hit, aggregate, and store the largest radius hit
                std::vector<float> position = simHit->getPosition();
                hcalXpos[detIDraw]  = position[0]* simHit->getEdep();
                hcalYpos[detIDraw]  = position[1]* simHit->getEdep();
                hcalZpos[detIDraw]  = position[2]* simHit->getEdep();
                hcalLayerEdep[detIDraw] += simHit->getEdep();
                hcalLayerTime[detIDraw] += simHit->getTime() * simHit->getEdep();
            }
        } 


        // loop over detID (layers) and simulate number of PEs
        int ihit = 0;
        for (std::map<int, float>::iterator it = hcalLayerEdep.begin(); it != hcalLayerEdep.end(); ++it) {
            int detIDraw = it->first;

            double depEnergy = hcalLayerEdep[detIDraw];
            hcalLayerTime[detIDraw] = hcalLayerTime[detIDraw] / hcalLayerEdep[detIDraw];
            hcalXpos[detIDraw] = hcalXpos[detIDraw] / hcalLayerEdep[detIDraw];
            hcalYpos[detIDraw] = hcalYpos[detIDraw] / hcalLayerEdep[detIDraw];
            hcalZpos[detIDraw] = hcalZpos[detIDraw] / hcalLayerEdep[detIDraw];
            double meanPE = depEnergy / MEV_PER_MIP * PE_PER_MIP;

            //        std::default_random_engine generator;
            //std::poisson_distribution<int> distribution(meanPE);

            hcalLayerPEs[detIDraw] = random_->Poisson(meanPE);
            hcalLayerPEs[detIDraw] += random_->Gaus(meanNoise_);

            if (verbose_) {
                detID_->setRawValue(detIDraw);
                detID_->unpack();
                int layer = detID_->getFieldValue("layer");
                int subsection = detID_->getFieldValue("subsection");
                int strip = detID_->getFieldValue("strip");

                std::cout << "detID: " << detIDraw << std::endl;
                std::cout << "Layer: " << layer << std::endl;
                std::cout << "Subsection: " << subsection << std::endl;
                std::cout << "Strip: " << strip << std::endl;
                std::cout << "Edep: " << hcalLayerEdep[detIDraw] << std::endl;
                std::cout << "numPEs: " << hcalLayerPEs[detIDraw] << std::endl;
                std::cout << "time: " << hcalLayerTime[detIDraw] << std::endl;
                std::cout << "z: " << hcalZpos[detIDraw] << std::endl;
            }        // end verbose

            double energy = hcalLayerPEs[detIDraw] / PE_PER_MIP * MEV_PER_MIP; // need to add in a weighting factor eventually

            HcalStripHit *hit = (HcalStripHit*) (hits_->ConstructedAt(ihit));

            //	hit->setLayer(layer);
            hit->setPE(hcalLayerPEs[detIDraw]);
            hit->setAmplitude(hcalLayerPEs[detIDraw]);
            hit->setEnergy(energy);
            hit->setTime(hcalLayerTime[detIDraw]);
            hit->setID(detIDraw);
            hit->setXpos(hcalXpos[detIDraw]);
            hit->setYpos(hcalYpos[detIDraw]);
            hit->setZpos(hcalZpos[detIDraw]);
            ihit++;
        } 
        
        
        event.add("hcalStripDigis", hits_);
    }

}

DECLARE_PRODUCER_NS(ldmx, HcalStripDigiProducer);

