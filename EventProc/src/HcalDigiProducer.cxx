#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "EventProc/HcalDigiProducer.h"

#include <iostream>

namespace ldmx {

    HcalDigiProducer::HcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
        hits_ = new TClonesArray(EventConstants::HCAL_HIT.c_str());
        noiseGenerator_ = new NoiseGenerator();
    }

    void HcalDigiProducer::configure(const ParameterSet& ps) {
        detID_       = new HcalID();
        random_      = new TRandom3(ps.getInteger("randomSeed", 1000));
        STRIPS_BACK_PER_LAYER_ = ps.getInteger("strips_back_per_layer");
        NUM_BACK_HCAL_LAYERS_ = ps.getInteger("num_back_hcal_layers");
        STRIPS_SIDE_TB_PER_LAYER_ = ps.getInteger("strips_side_tb_per_layer");
        NUM_SIDE_TB_HCAL_LAYERS_ = ps.getInteger("num_side_tb_hcal_layers");
        STRIPS_SIDE_LR_PER_LAYER_ = ps.getInteger("strips_side_lr_per_layer");
        NUM_SIDE_LR_HCAL_LAYERS_ = ps.getInteger("num_side_lr_hcal_layers");
        SUPER_STRIP_SIZE_ = ps.getInteger("super_strip_size");
        readoutThreshold_ = ps.getInteger("readoutThreshold");
        meanNoise_   = ps.getDouble("meanNoise");
        mev_per_mip_ = ps.getDouble("mev_per_mip");
        pe_per_mip_  = ps.getDouble("pe_per_mip");
        doStrip_     = ps.getInteger("doStrip");
        noiseGenerator_ = new NoiseGenerator(meanNoise_,false);
        noiseGenerator_->setNoiseThreshold(readoutThreshold_);
    }

    unsigned int HcalDigiProducer::generateRandomID(HcalSection sec){
        HcalID tempID;
        if( sec == HcalSection::BACK ){
            tempID.setFieldValue(1,random_->Integer(NUM_BACK_HCAL_LAYERS_));
            tempID.setFieldValue(2,0);
            tempID.setFieldValue(3,random_->Integer(STRIPS_BACK_PER_LAYER_));
        }else if( sec == HcalSection::TOP || sec == HcalSection::BOTTOM ){
            tempID.setFieldValue(1,random_->Integer(NUM_SIDE_TB_HCAL_LAYERS_));
            tempID.setFieldValue(2,random_->Integer(2)+1);
            tempID.setFieldValue(3,random_->Integer(STRIPS_SIDE_TB_PER_LAYER_));            
        }else if( sec == HcalSection::LEFT || sec == HcalSection::RIGHT ){
            tempID.setFieldValue(1,random_->Integer(NUM_SIDE_LR_HCAL_LAYERS_));
            tempID.setFieldValue(2,random_->Integer(2)+3);
            tempID.setFieldValue(3,random_->Integer(STRIPS_SIDE_LR_PER_LAYER_));            
	}else
	    std::cout << "WARNING [HcalDigiProducer::generateRandomID]: HcalSection is not known" << std::endl;

        return tempID.pack();
    }

    void HcalDigiProducer::produce(Event& event) {

        std::map<unsigned int, int>   hcalLayerPEs;
        std::map<unsigned int, float> hcalXpos,hcalYpos,hcalZpos,hcaldetIDEdep, hcaldetIDTime;
        int numSigHits_back=0,numSigHits_side_tb=0,numSigHits_side_lr=0;
        std::unordered_set<unsigned int> noiseHitIDs;

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
                std::cout << "----" << std::endl;
                std::cout << "before detIDraw: " << detIDraw << std::endl;
                detID_->setRawValue(detIDraw);
                detID_->unpack();
                int layer = detID_->getFieldValue("layer");
                int subsection = detID_->getFieldValue("section");
                int strip = detID_->getFieldValue("strip");                
                std::cout << "section: " << subsection << "  layer: " << layer <<  "  strip: " << strip <<std::endl;
                detID_->setFieldValue("strip",33);
                detID_->pack();
                std::cout << "after, detIDraw: " << detID_->getRawValue() << std::endl;
                detID_->unpack();
                std::cout << "section: " << detID_->getFieldValue("section") << "  layer: " << detID_->getFieldValue("layer") <<  "  strip: " << detID_->getFieldValue("strip") <<std::endl;
            }        

            // re-assign the strip number based on super strip size
            if (SUPER_STRIP_SIZE_ != 1){
                detID_->setRawValue(detIDraw);
                detID_->unpack();
                int strip = detID_->getFieldValue("strip");                
                int newstrip = strip/SUPER_STRIP_SIZE_;
                detID_->setFieldValue("strip",newstrip);
                detID_->pack();
                // get the new raw value
                detIDraw = detID_->getRawValue();
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
        for (std::map<unsigned int, float>::iterator it = hcaldetIDEdep.begin(); it != hcaldetIDEdep.end(); ++it) {
            int detIDraw = it->first;
            double depEnergy = hcaldetIDEdep[detIDraw];
            hcaldetIDTime[detIDraw] = hcaldetIDTime[detIDraw] / hcaldetIDEdep[detIDraw];
            hcalXpos[detIDraw]      = hcalXpos[detIDraw] / hcaldetIDEdep[detIDraw];
            hcalYpos[detIDraw]      = hcalYpos[detIDraw] / hcaldetIDEdep[detIDraw];
            hcalZpos[detIDraw]      = hcalZpos[detIDraw] / hcaldetIDEdep[detIDraw];
            double meanPE           = depEnergy / mev_per_mip_ * pe_per_mip_;

            HcalID tempID;
            tempID.setRawValue(detIDraw);
            tempID.unpack();
            if( tempID.getSection() == HcalSection::BACK )
                numSigHits_back++;
            else if( tempID.getSection() == HcalSection::TOP || tempID.getSection() == HcalSection::BOTTOM )
                numSigHits_side_tb++;
            else if( tempID.getSection() == HcalSection::LEFT || tempID.getSection() == HcalSection::RIGHT )
                numSigHits_side_lr++;
	        else std::cout << "WARNING [HcalDigiProducer::produce]: HcalSection is not known" << std::endl;
	    
            hcalLayerPEs[detIDraw] = random_->Poisson(meanPE+meanNoise_);
            //hcalLayerPEs[detIDraw] += random_->Gaus(meanNoise_);

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
                std::cout << "Layer: " << layer << "\t Strip: " << strip << "\t X: " << hcalXpos[detIDraw] <<  "\t Y: " << hcalYpos[detIDraw] <<  "\t Z: " << hcalZpos[detIDraw] << std::endl;
            }        // end verbose

            // need to add in a weighting factor eventually, so keep it that way to make sure
            // we don't forget about it
            double energy = depEnergy; 

            // quantize/smear the position
            detID_->setRawValue(detIDraw);
            detID_->unpack();
            int cur_subsection = detID_->getFieldValue("section");            
            int cur_layer      = detID_->getFieldValue("layer");
            int cur_strip      = detID_->getFieldValue("strip");
            float cur_xpos, cur_ypos; 
            if (cur_subsection == 0){
                float super_strip_width = SUPER_STRIP_SIZE_*50.0;
                float total_super_strips = STRIPS_BACK_PER_LAYER_/SUPER_STRIP_SIZE_;
                float total_width = super_strip_width*total_super_strips;
                if (cur_layer % 2 == 0){ // even layers
                    cur_xpos = (super_strip_width * (float(cur_strip)+0.5)) - total_width/2.; 
                    cur_ypos = hcalYpos[detIDraw] + random_->Gaus(150.); 
                }
                if (cur_layer % 2 == 1){ // odd layers
                    cur_ypos = (super_strip_width * (float(cur_strip)+0.5)) - total_width/2.; 
                    cur_xpos = hcalXpos[detIDraw] + random_->Gaus(150.); 
                }
                if (cur_xpos > total_width/2.) cur_xpos = total_width/2.;
                if (cur_xpos < -1.*total_width/2.) cur_xpos = -1.*total_width/2.;
                if (cur_ypos > total_width/2.) cur_ypos = total_width/2.;
                if (cur_ypos < -1.*total_width/2.) cur_ypos = -1.*total_width/2.;
            }
            std::cout << "Layer = " << cur_layer << "\t Section = " << cur_subsection << "\t Strip = " << cur_strip << "\t Xpos = " << cur_xpos << "\t Ypos = " << cur_ypos << "\t X weighted position = " << hcalXpos[detIDraw] << "\t Y weighted position = " << hcalYpos[detIDraw] << std::endl;

            if( hcalLayerPEs[detIDraw] >= readoutThreshold_ ){ // > or >= ?
                HcalHit *hit = (HcalHit*) (hits_->ConstructedAt(ihit));
                
                hit->setID(detIDraw);
                hit->setPE(hcalLayerPEs[detIDraw]);
                hit->setAmplitude(hcalLayerPEs[detIDraw]);
                hit->setEnergy(energy);
                hit->setTime(hcaldetIDTime[detIDraw]);
                hit->setXpos(hcalXpos[detIDraw]);
                hit->setYpos(hcalYpos[detIDraw]);
                hit->setZpos(hcalZpos[detIDraw]);
                hit->setNoise(false);
                ihit++;
                
            }
        } 
        
        // ------------------------------- Noise simulation -------------------------------
        // simulate noise hits in back hcal
        std::vector<double> noiseHits_PE = noiseGenerator_->generateNoiseHits(STRIPS_BACK_PER_LAYER_*NUM_BACK_HCAL_LAYERS_-numSigHits_back);
        for( auto noise : noiseHits_PE ){
            HcalHit* noiseHit = (HcalHit*) (hits_->ConstructedAt(ihit));
            noiseHit->setPE(noise);
            noiseHit->setAmplitude(noise);
            noiseHit->setXpos(0.);
            noiseHit->setYpos(0.);
            noiseHit->setZpos(0.);
            noiseHit->setTime(-999.);
            noiseHit->setEnergy(noise*mev_per_mip_/pe_per_mip_);
            unsigned int rawID;
            do{
	      rawID = generateRandomID(HcalSection::BACK);
            }while( hcaldetIDEdep.find(rawID) != hcaldetIDEdep.end() || noiseHitIDs.find(rawID) != noiseHitIDs.end() );
            noiseHit->setID(rawID);
            noiseHitIDs.insert(rawID);
            noiseHit->setNoise(true);
            ihit++;
        }

        // simulate noise hits in side, top/bottom hcal
        noiseHits_PE = noiseGenerator_->generateNoiseHits((STRIPS_SIDE_TB_PER_LAYER_*NUM_SIDE_TB_HCAL_LAYERS_)*2-numSigHits_side_tb);
        for( auto noise : noiseHits_PE ){
            HcalHit* noiseHit = (HcalHit*) (hits_->ConstructedAt(ihit));
            noiseHit->setPE(noise);
            noiseHit->setAmplitude(noise);
            noiseHit->setXpos(0.);
            noiseHit->setYpos(0.);
            noiseHit->setZpos(0.);
            noiseHit->setTime(-999.);
            noiseHit->setEnergy(noise*mev_per_mip_/pe_per_mip_);
            unsigned int rawID;
            do{
	        rawID = generateRandomID(HcalSection::TOP);
            }while( hcaldetIDEdep.find(rawID) != hcaldetIDEdep.end() || noiseHitIDs.find(rawID) != noiseHitIDs.end() );
            noiseHit->setID(rawID);
            noiseHitIDs.insert(rawID);
            noiseHit->setNoise(true);
            ihit++;
        }

        // simulate noise hits in side, left/right hcal
        noiseHits_PE = noiseGenerator_->generateNoiseHits((STRIPS_SIDE_LR_PER_LAYER_*NUM_SIDE_LR_HCAL_LAYERS_)*2-numSigHits_side_lr);
        for( auto noise : noiseHits_PE ){
            HcalHit* noiseHit = (HcalHit*) (hits_->ConstructedAt(ihit));
            noiseHit->setPE(noise);
            noiseHit->setAmplitude(noise);
            noiseHit->setXpos(0.);
            noiseHit->setYpos(0.);
            noiseHit->setZpos(0.);
            noiseHit->setTime(-999.);
            noiseHit->setEnergy(noise*mev_per_mip_/pe_per_mip_);
            unsigned int rawID;
            do{
	        rawID = generateRandomID(HcalSection::LEFT);
            }while( hcaldetIDEdep.find(rawID) != hcaldetIDEdep.end() || noiseHitIDs.find(rawID) != noiseHitIDs.end() );
            noiseHit->setID(rawID);
            noiseHitIDs.insert(rawID);
            noiseHit->setNoise(true);
            ihit++;
        }

        event.add("hcalDigis", hits_);
    }

}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);

