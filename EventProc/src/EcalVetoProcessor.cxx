#include "EventProc/EcalVetoProcessor.h"

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/Event.h"
#include <algorithm>

namespace ldmx {

    void EcalVetoProcessor::configure(const ParameterSet& ps) {
        hexReadout_ = new EcalHexReadout();

        nEcalLayers_ = ps.getInteger("num_ecal_layers");
        backEcalStartingLayer_ = ps.getInteger("back_ecal_starting_layer");
        nBDTVars_ = ps.getInteger("n_bdt_vars");
        if (nBDTVars_ != 0){
        	BDTHelper_ = new BDTHelper("",nBDTVars_);
        }
        bdtCutVal_ = ps.getDouble("discCut");
        EcalLayerEdepRaw_.resize(nEcalLayers_, 0);
        EcalLayerEdepReadout_.resize(nEcalLayers_, 0);
        EcalLayerOuterRaw_.resize(nEcalLayers_, 0);
        EcalLayerOuterReadout_.resize(nEcalLayers_, 0);
        EcalLayerTime_.resize(nEcalLayers_, 0);
        cellMap_.resize(nEcalLayers_,std::map<int,float>());
        cellMapLooseIso_.resize(nEcalLayers_,std::map<int,float>());
        cellMapTightIso_.resize(nEcalLayers_,std::map<int,float>());


    }

    void EcalVetoProcessor::produce(Event& event) {
    	for (int i = 0; i < nEcalLayers_; i++){
    		cellMap_[i].clear();
    		cellMapLooseIso_[i].clear();
    		cellMapTightIso_[i].clear();
    	}
    	/* New expanded variable collection to help with background veto */
    	/* Loose isolated hits -- Any hit with no readout in nearest neighbors */
    	/* Any isolated hit that does have the event centroid as a nearest neighbor */
    	/* Loose tracks -- two consecutive layers w/ projected nearest neighbor loose isolated hits */
    	/* Medium tracks -- three consecutive layers w/ projected nearest neighbor loose isolated hits */
    	/* Tight tracks -- three consecutive layers w/ projected nearest neighbor tight isolated hits */

    	looseMipTracks_.clear();
    	mediumMipTracks_.clear();
    	tightMipTracks_.clear();
	bdtFeatures_.clear();

    	nReadoutHits_  	 = 0;
    	nLooseIsoHits_ 	 = 0;
    	nTightIsoHits_   = 0;
    	summedDet_ 		 = 0;
    	summedOuter_ 	 = 0;
    	backSummedDet_ 	 = 0;
    	summedLooseIso_  = 0;
    	maxLooseIsoDep_  = 0;
    	summedTightIso_  = 0;
    	maxTightIsoDep_  = 0;
    	maxCellDep_		 = 0;
    	showerRMS_		 = 0;

        std::fill(EcalLayerEdepRaw_.begin(), EcalLayerEdepRaw_.end(), 0);
        std::fill(EcalLayerEdepReadout_.begin(), EcalLayerEdepReadout_.end(), 0);
        std::fill(EcalLayerOuterRaw_.begin(), EcalLayerOuterRaw_.end(), 0);
        std::fill(EcalLayerOuterReadout_.begin(), EcalLayerOuterReadout_.end(), 0);
        std::fill(EcalLayerTime_.begin(), EcalLayerTime_.end(), 0); 

        // Get the collection of digitized Ecal hits from the event. 
        const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
        int nEcalHits = ecalDigis->GetEntriesFast();

        std::cout << "[ EcalVetoProcessor ] : Got " << nEcalHits << " ECal digis in event " << event.getEventHeader()->getEventNumber() << std::endl;

        int globalCentroid = GetShowerCentroidIDAndRMS(ecalDigis,showerRMS_);
        /* ~~ Fill the hit map ~~ O(n)  */
        fillHitMap(ecalDigis,cellMap_);
        bool doTight = true;
        /* ~~ Fill the isolated hit maps ~~ O(n)  */
        fillIsolatedHitMap(ecalDigis,globalCentroid,cellMap_,cellMapTightIso_,doTight);
        fillIsolatedHitMap(ecalDigis,globalCentroid,cellMap_,cellMapLooseIso_,!doTight);
        int trackLen = 2;


        //Loop over the hits from the event to calculate the rest of the important quantities
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            //Layer-wise quantities
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            EcalLayerEdepRaw_[hit_pair.first] = EcalLayerEdepRaw_[hit_pair.first] + hit->getEnergy();
            if (maxCellDep_ < hit->getEnergy()) maxCellDep_ = hit->getEnergy();
            if (hit->getEnergy() > 0) {
            	nReadoutHits_++;
                EcalLayerEdepReadout_[hit_pair.first] += hit->getEnergy();
                EcalLayerTime_[hit_pair.first] += (hit->getEnergy()) * hit->getTime();
            }
            //Check Outer
            if (!(hexReadout_->isInShowerInnerRing(globalCentroid, hit_pair.second)) && !(hexReadout_->isInShowerOuterRing(globalCentroid, hit_pair.second)) && !(hit_pair.second == globalCentroid)) {

                EcalLayerOuterRaw_[hit_pair.first] += hit->getEnergy();

                if (hit->getEnergy() > 0)
                    EcalLayerOuterReadout_[hit_pair.first] += hit->getEnergy();
            }
        }

        // end loop over sim hits

        for (int iLayer = 0; iLayer < EcalLayerEdepReadout_.size(); iLayer++) {
    		for (auto cell : cellMapLooseIso_[iLayer]){
    			if (cell.second > 0){
    				nLooseIsoHits_++;
    				summedLooseIso_ += cell.second;
    			}

    			if (cell.second > maxLooseIsoDep_){
    				maxLooseIsoDep_ = cell.second;
    			}
    		}
    		for (auto cell : cellMapTightIso_[iLayer]){
    			if (cell.second > 0){
    				nTightIsoHits_++;
    				summedTightIso_ += cell.second;
    			}

    			if (cell.second > maxTightIsoDep_){
    				maxTightIsoDep_ = cell.second;
    			}
    		}
    		EcalLayerTime_[iLayer] = EcalLayerTime_[iLayer] / EcalLayerEdepReadout_[iLayer];
            summedDet_ 			  += EcalLayerEdepReadout_[iLayer];
            summedOuter_ 		  += EcalLayerOuterReadout_[iLayer];
            if (iLayer > backEcalStartingLayer_)
                backSummedDet_ 	  += EcalLayerEdepReadout_[iLayer];
        }

        /*std::cout << "[ EcalVetoProcessor ]:\n" 
          << "\t EdepRaw[0] : " << EcalLayerEdepRaw_[0] << "\n"
          << "\t EdepReadout[0] : " << EcalLayerEdepReadout_[0] << "\n"
          << "\t EdepLayerOuterRaw[0] : " << EcalLayerOuterRaw_[0] << "\n"
          << "\t EdepLayerOuterReadout[0] : " << EcalLayerOuterReadout_[0] << "\n"
          << "\t EdepLayerTime[0] : " << EcalLayerTime_[0] << "\n"
          << "\t Shower Median: " << showerMedianCellId 
          << std::endl;*/

        /* ~~ Fill the mip tracks ~~ O(n_iso^2)  */
        std::vector<std::map<int,float>> looseIsoCopy(cellMapLooseIso_);
        fillMipTracks(cellMapLooseIso_,looseMipTracks_,trackLen);
        trackLen = 3;
        fillMipTracks(looseIsoCopy,mediumMipTracks_,trackLen);
        fillMipTracks(cellMapTightIso_,tightMipTracks_,trackLen);

        result_.setVariables(nReadoutHits_,nLooseIsoHits_,nTightIsoHits_,
                                summedDet_,summedOuter_,backSummedDet_,
                                summedLooseIso_,maxLooseIsoDep_,
                                summedTightIso_,maxTightIsoDep_,
                                maxCellDep_,showerRMS_,
                        EcalLayerEdepReadout_,looseMipTracks_,mediumMipTracks_,tightMipTracks_);
        BDTHelper_->buildFeatureVector(bdtFeatures_,result_);
    	double pred = BDTHelper_->getSinglePred(bdtFeatures_);

        result_.setVetoResult(pred > bdtCutVal_);
        event.addToCollection("EcalVeto", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
