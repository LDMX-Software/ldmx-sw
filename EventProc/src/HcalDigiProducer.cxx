#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/HcalHit.h"
#include "EventProc/HcalDigiProducer.h"

#include <iostream>

#include "DetDescr/HcalID.h"
#include "Event/HcalHit.h"

namespace ldmx {

    HcalDigiProducer::HcalDigiProducer(const std::string& name, Process& process) :
            Producer(name, process) {
        hits_ = new TClonesArray(EventConstants::HCAL_HIT.c_str());
    }

    void HcalDigiProducer::configure(const ParameterSet& ps) {
        detID_       = new HcalID();
        random_      = new TRandom(ps.getInteger("randomSeed", 1000));
        meanNoise_   = ps.getDouble("meanNoise");
        mev_per_mip_ = ps.getDouble("mev_per_mip");
        pe_per_mip_  = ps.getDouble("pe_per_mip");
        doStrip_     = ps.getInteger("doStrip");
    }

    void HcalDigiProducer::produce(Event& event) {

        std::map<int, int>   hcalLayerPEs;
        std::map<int, float> hcalXpos,hcalYpos,hcalZpos,hcaldetIDEdep, hcaldetIDTime;

        // looper over sim hits and aggregate energy depositions for each detID
        TClonesArray* hcalHits = (TClonesArray*) event.getCollection(EventConstants::HCAL_SIM_HITS, "sim");

        int numHCalSimHits = hcalHits->GetEntries();
        for (int iHit = 0; iHit < numHCalSimHits; iHit++) {
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) hcalHits->At(iHit);
            int detIDraw = simHit->getID();
            std::vector<float> position = simHit->getPosition();
	    
	    //if we aggregate by layer, set all strip to zero and rcalculate the detIDraw
	    if (doStrip_ == 0){
                detID_->setRawValue(detIDraw);
	        detID_->unpack();
	        detID_->setFieldValue(3,0);
                detIDraw = detID_->pack();
	    }           

            if (verbose_) {
                std::cout << "detIDraw: " << detIDraw << std::endl;
                detID_->setRawValue(detIDraw);
                detID_->unpack();
                int layer = detID_->getFieldValue("layer");
                int subsection = detID_->getFieldValue("section");
                int strip = detID_->getFieldValue("strip");                
                std::cout << "section: " << subsection << "  layer: " << layer <<  "  strip: " << strip <<std::endl;
            }           
            
	    // for now, we take am energy weighted average of the hit in each stip to simulate the hit position. 
            // will use strip TOF and light yield between strips to estimate position.            
	    if (hcaldetIDEdep.find(detIDraw) == hcaldetIDEdep.end()) {
                // first hit, initialize
                hcaldetIDEdep[detIDraw] = simHit->getEdep();
                hcaldetIDTime[detIDraw] = simHit->getTime() * simHit->getEdep();
                hcalXpos[detIDraw]      = position[0]* simHit->getEdep();
                hcalYpos[detIDraw]      = position[1]* simHit->getEdep();
                hcalZpos[detIDraw]      = position[2]* simHit->getEdep();
            } else {
                // not first hit, aggregate, and store the largest radius hit
                hcalXpos[detIDraw]      += position[0]* simHit->getEdep();
                hcalYpos[detIDraw]      += position[1]* simHit->getEdep();
                hcalZpos[detIDraw]      += position[2]* simHit->getEdep();
                hcaldetIDEdep[detIDraw] += simHit->getEdep();
                hcaldetIDTime[detIDraw] += simHit->getTime() * simHit->getEdep();
            
	    }
	    
        } 


        // loop over detIDs and simulate number of PEs
        int ihit = 0;
        for (std::map<int, float>::iterator it = hcaldetIDEdep.begin(); it != hcaldetIDEdep.end(); ++it) {
            int detIDraw = it->first;
            double depEnergy = hcaldetIDEdep[detIDraw];
            hcaldetIDTime[detIDraw] = hcaldetIDTime[detIDraw] / hcaldetIDEdep[detIDraw];
            hcalXpos[detIDraw]      = hcalXpos[detIDraw] / hcaldetIDEdep[detIDraw];
            hcalYpos[detIDraw]      = hcalYpos[detIDraw] / hcaldetIDEdep[detIDraw];
            hcalZpos[detIDraw]      = hcalZpos[detIDraw] / hcaldetIDEdep[detIDraw];
            double meanPE           = depEnergy / mev_per_mip_ * pe_per_mip_;

            //std::default_random_engine generator;
            //std::poisson_distribution<int> distribution(meanPE);
            hcalLayerPEs[detIDraw] = random_->Poisson(meanPE);
            hcalLayerPEs[detIDraw] += random_->Gaus(meanNoise_);

            if (verbose_) {
                detID_->setRawValue(detIDraw);
                detID_->unpack();
                int layer = detID_->getFieldValue("layer");
                int subsection = detID_->getFieldValue("section");
                int strip = detID_->getFieldValue("strip");

                std::cout << "detID: " << detIDraw << std::endl;
                std::cout << "Layer: " << layer << std::endl;
                std::cout << "Subsection: " << subsection << std::endl;
                std::cout << "Strip: " << strip << std::endl;
                std::cout << "Edep: " << hcaldetIDEdep[detIDraw] << std::endl;
                std::cout << "numPEs: " << hcalLayerPEs[detIDraw] << std::endl;
                std::cout << "time: " << hcaldetIDTime[detIDraw] << std::endl;
                std::cout << "z: " << hcalZpos[detIDraw] << std::endl;
            }        // end verbose

	    // need to add in a weighting factor eventually, so keep it that way to make sure
	    // we don't forget about it
            double energy = depEnergy; 

            HcalHit *hit = (HcalHit*) (hits_->ConstructedAt(ihit));

            hit->setID(detIDraw);
            hit->setPE(hcalLayerPEs[detIDraw]);
            hit->setAmplitude(hcalLayerPEs[detIDraw]);
            hit->setEnergy(energy);
            hit->setTime(hcaldetIDTime[detIDraw]);
            hit->setXpos(hcalXpos[detIDraw]);
            hit->setYpos(hcalYpos[detIDraw]);
            hit->setZpos(hcalZpos[detIDraw]);
            ihit++;
        } 
        
        
        event.add("hcalDigis", hits_);
    }

}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);

