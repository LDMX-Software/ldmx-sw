#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "EventProc/HcalDigiProducer.h"

#include <iostream>
#include <exception>

namespace ldmx {

    HcalDigiProducer::HcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
        hits_ = new TClonesArray(EventConstants::HCAL_HIT.c_str());
        noiseGenerator_ = new NoiseGenerator();
    }

    void HcalDigiProducer::configure(const ParameterSet& ps) {
        detID_       = new HcalID();
        random_      = new TRandom3(ps.getInteger("randomSeed", 1000));
        STRIPS_BACK_PER_LAYER_     = ps.getInteger("strips_back_per_layer");
        NUM_BACK_HCAL_LAYERS_      = ps.getInteger("num_back_hcal_layers");
        STRIPS_SIDE_TB_PER_LAYER_  = ps.getInteger("strips_side_tb_per_layer");
        NUM_SIDE_TB_HCAL_LAYERS_   = ps.getInteger("num_side_tb_hcal_layers");
        STRIPS_SIDE_LR_PER_LAYER_  = ps.getInteger("strips_side_lr_per_layer");
        NUM_SIDE_LR_HCAL_LAYERS_   = ps.getInteger("num_side_lr_hcal_layers");
        SUPER_STRIP_SIZE_          = ps.getInteger("super_strip_size");
        readoutThreshold_          = ps.getInteger("readoutThreshold");
        meanNoise_                 = ps.getDouble("meanNoise");
        mev_per_mip_               = ps.getDouble("mev_per_mip");
        pe_per_mip_                = ps.getDouble("pe_per_mip");
        strip_attenuation_length_  = ps.getDouble("strip_attenuation_length");
        strip_position_resolution_  = ps.getDouble("strip_position_resolution");
        noiseGenerator_ = new NoiseGenerator(meanNoise_,false);
        //noiseGenerator_->setNoiseThreshold(readoutThreshold_);
        noiseGenerator_->setNoiseThreshold(1); // hard-code this number, create noise hits for non-zero PEs! 
    }

    unsigned int HcalDigiProducer::generateRandomID(HcalSection sec){
        HcalID tempID;
        if( sec == HcalSection::BACK ){
            tempID.setFieldValue(1,random_->Integer(NUM_BACK_HCAL_LAYERS_));
            tempID.setFieldValue(2,0);
            //tempID.setFieldValue(3,random_->Integer(STRIPS_BACK_PER_LAYER_));
            tempID.setFieldValue(3,random_->Integer(STRIPS_BACK_PER_LAYER_/SUPER_STRIP_SIZE_));
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
        std::map<unsigned int, int>   hcalLayerMinPEs;
        std::map<unsigned int, float> hcalXpos,hcalYpos,hcalZpos,hcaldetIDEdep, hcaldetIDTime;
        int numSigHits_back=0,numSigHits_side_tb=0,numSigHits_side_lr=0;
        std::unordered_set<unsigned int> noiseHitIDs;

        // first check if the super strip size divides nicely into the total number of strips
        if (STRIPS_BACK_PER_LAYER_ % SUPER_STRIP_SIZE_ != 0){
            throw std::invalid_argument( "HcalDigiProducer: the specified superstrip size is not compatible with total number of strips!" );
        }

        // looper over sim hits and aggregate energy depositions for each detID
        TClonesArray* hcalHits = (TClonesArray*) event.getCollection(EventConstants::HCAL_SIM_HITS, "sim");

        int numHCalSimHits = hcalHits->GetEntries();
        for (int iHit = 0; iHit < numHCalSimHits; iHit++) {
            
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) hcalHits->At(iHit);
            int detIDraw = simHit->getID();
            detID_->setRawValue(detIDraw);
            detID_->unpack();
            int layer = detID_->getFieldValue("layer");
            int subsection = detID_->getFieldValue("section");
            int strip = detID_->getFieldValue("strip");                 
            std::vector<float> position = simHit->getPosition();       

            if (verbose_) {
                std::cout << "section: " << detID_->getFieldValue("section") << "  layer: " << detID_->getFieldValue("layer") <<  "  strip: " << detID_->getFieldValue("strip") <<std::endl;
            }        

            // re-assign the strip number based on super strip size -- ONLY FOR Back Hcal
            // int subsection = detID_->getFieldValue("section");
            if (SUPER_STRIP_SIZE_ != 1 && subsection == 0){
                int detIDraw_orig = detIDraw;
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

            // need to add in a weighting factor eventually, so keep it that way to make sure
            // we don't forget about it
            double energy = depEnergy; 

            // quantize/smear the position
            HcalID *curDetId = new HcalID();
            curDetId->setRawValue(detIDraw);
            curDetId->unpack();
            int cur_subsection = curDetId->getFieldValue("section");            
            int cur_layer      = curDetId->getFieldValue("layer");
            int cur_strip      = curDetId->getFieldValue("strip");
            float cur_xpos, cur_ypos; 

            if (cur_subsection != 0){ // for sidecal don't worry about attenuation because it's single readout
                hcalLayerPEs[detIDraw] = random_->Poisson(meanPE+meanNoise_);
                hcalLayerMinPEs[detIDraw] = hcalLayerPEs[detIDraw];
            }
            if (cur_subsection == 0){// get PEs with attentuation
                meanPE *= exp(1./strip_attenuation_length_); // increase the PE count to the case with no attentuation (assuming 80% attenuation on the pe_per_mip number @ 1m)
                float total_width = STRIPS_BACK_PER_LAYER_*50.0;
                float distance_along_bar = 0;
                if (cur_layer % 2 == 0){
                    distance_along_bar = fabs(cur_ypos);
                }
                if (cur_layer % 2 == 1){
                    distance_along_bar = fabs(cur_xpos);
                }
                float meanPE_close = meanPE * exp( -1. * ((total_width/2. - distance_along_bar) / 1000.) / strip_attenuation_length_ );
                float meanPE_far   = meanPE * exp( -1. * ((total_width/2. + distance_along_bar) / 1000.) / strip_attenuation_length_ );
                float PE_close     = random_->Poisson(meanPE_close+meanNoise_);
                float PE_far       = random_->Poisson(meanPE_far+meanNoise_);
                hcalLayerPEs[detIDraw] = PE_close + PE_far;
                hcalLayerMinPEs[detIDraw] = std::min(PE_close,PE_far);
            }
            // std::cout << "depEnergy = " << depEnergy << "\t hcalLayerPEs[detIDraw] = " << hcalLayerPEs[detIDraw] << "\t hcalLayerMinPEs[detIDraw] = " << hcalLayerMinPEs[detIDraw] << std::endl;

            if (cur_subsection == 0){
                float super_strip_width = SUPER_STRIP_SIZE_*50.0;
                float total_super_strips = STRIPS_BACK_PER_LAYER_/SUPER_STRIP_SIZE_; // to help the integer round up
                float total_width = STRIPS_BACK_PER_LAYER_*50.0;
                if (cur_layer % 2 == 0){ // even layers, vertical
                    cur_xpos = (super_strip_width * (float(cur_strip)+0.5)) - total_width/2.; 
                    cur_ypos = hcalYpos[detIDraw] + random_->Gaus(0.,strip_position_resolution_); 
                }
                if (cur_layer % 2 == 1){ // odd layers, horizontal
                    cur_ypos = (super_strip_width * (float(cur_strip)+0.5)) - total_width/2.; 
                    cur_xpos = hcalXpos[detIDraw] + random_->Gaus(0.,strip_position_resolution_); 
                }
                if (cur_xpos > total_width/2.) cur_xpos = total_width/2.;
                if (cur_xpos < -1.*total_width/2.) cur_xpos = -1.*total_width/2.;
                if (cur_ypos > total_width/2.) cur_ypos = total_width/2.;
                if (cur_ypos < -1.*total_width/2.) cur_ypos = -1.*total_width/2.;
                // std::cout << "super_strip_width = " << super_strip_width << "\t total_super_strips = " << total_super_strips << "\t total_width = " << total_width << std::endl;
            }

            if( hcalLayerPEs[detIDraw] >= readoutThreshold_ ){ // > or >= ?
                
                HcalHit *hit = (HcalHit*) (hits_->ConstructedAt(ihit));
                
                hit->setID(detIDraw);
                hit->setPE(hcalLayerPEs[detIDraw]);
                hit->setMinPE(hcalLayerMinPEs[detIDraw]);
                hit->setAmplitude(hcalLayerPEs[detIDraw]);
                hit->setEnergy(energy);
                hit->setTime(hcaldetIDTime[detIDraw]);
                // hit->setXpos(hcalXpos[detIDraw]);
                // hit->setYpos(hcalYpos[detIDraw]);
                hit->setXpos(cur_xpos); // quantized and smeared positions
                hit->setYpos(cur_ypos); // quantized and smeared positions
                hit->setZpos(hcalZpos[detIDraw]);
                hit->setNoise(false);
                ihit++;
                
            }

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
        } 
        

        // ------------------------------- Noise simulation -------------------------------
        // simulate noise hits in back hcal
        int total_super_strips_back = STRIPS_BACK_PER_LAYER_/SUPER_STRIP_SIZE_;
        int total_empty_channels = 2*(total_super_strips_back*NUM_BACK_HCAL_LAYERS_-numSigHits_back);
        std::vector<double> noiseHits_PE = noiseGenerator_->generateNoiseHits( total_empty_channels ); // 2-sided readout
        int total_zero_channels = total_empty_channels - noiseHits_PE.size();
        std::vector<double> zeroNoiseHits_PE(total_zero_channels, 0.0);
        noiseHits_PE.insert( noiseHits_PE.end(), zeroNoiseHits_PE.begin(), zeroNoiseHits_PE.end() );
        std::random_shuffle ( noiseHits_PE.begin(), noiseHits_PE.end() );
        // std::vector<double> noiseHits_PE_1 = noiseGenerator_->generateNoiseHits(total_super_strips_back*NUM_BACK_HCAL_LAYERS_-numSigHits_back);
        // std::vector<double> noiseHits_PE_2 = noiseGenerator_->generateNoiseHits(total_super_strips_back*NUM_BACK_HCAL_LAYERS_-numSigHits_back);
        // std::cout << "numSigHits_back = " << numSigHits_back << ", ihit = " << ihit << ", total_empty_channels = " << total_empty_channels << std::endl;
        int ctr_back_noise = 0;
        // //for( auto noise : noiseHits_PE ){
        // std::cout << "total_super_strips_back*NUM_BACK_HCAL_LAYERS_-numSigHits_back = " << (total_super_strips_back*NUM_BACK_HCAL_LAYERS_-numSigHits_back) << std::endl;
        for( unsigned int i = 0; i < noiseHits_PE.size()/2 ; ++i){
            double cur_noise_pe_1 = noiseHits_PE[i*2];
            double cur_noise_pe_2 = noiseHits_PE[i*2+1];
            double total_noise = cur_noise_pe_1 + cur_noise_pe_2;
            if (total_noise < readoutThreshold_) continue; // do nothing if the noise is 0

            HcalHit* noiseHit = (HcalHit*) (hits_->ConstructedAt(ihit));
            noiseHit->setPE(total_noise);
            noiseHit->setMinPE(std::min(cur_noise_pe_1,cur_noise_pe_2));
            noiseHit->setAmplitude(total_noise);
            noiseHit->setXpos(0.);
            noiseHit->setYpos(0.);
            noiseHit->setZpos(0.);
            noiseHit->setTime(-999.);
            noiseHit->setEnergy(total_noise*mev_per_mip_/pe_per_mip_);
            unsigned int rawID;
            do{
	           rawID = generateRandomID(HcalSection::BACK);
            } while( hcaldetIDEdep.find(rawID) != hcaldetIDEdep.end() || noiseHitIDs.find(rawID) != noiseHitIDs.end() );
            noiseHit->setID(rawID);
            noiseHitIDs.insert(rawID);
            noiseHit->setNoise(true);
            ihit++;
            ctr_back_noise++;
        }
        // std::cout << "numSigHits_back = " << numSigHits_back << ", ihit = " << ihit << ", ctr_back_noise = " << ctr_back_noise << std::endl;

        // simulate noise hits in side, top/bottom hcal
        noiseHits_PE = noiseGenerator_->generateNoiseHits((STRIPS_SIDE_TB_PER_LAYER_*NUM_SIDE_TB_HCAL_LAYERS_)*2-numSigHits_side_tb);
        for( auto noise : noiseHits_PE ){
            HcalHit* noiseHit = (HcalHit*) (hits_->ConstructedAt(ihit));
            noiseHit->setPE(noise);
            noiseHit->setMinPE(noise); // only one readout for sidecal
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
            noiseHit->setMinPE(noise); // only one readout for sidecal
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

