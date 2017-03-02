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
        nLayersMedCal_ = ps.getInteger("back_ecal_starting_layers");
        backEcalStartingLayer_ = ps.getInteger("num_layers_for_med_cal");
        totalDepCut_ = ps.getDouble("total_dep_cut");
        totalOuterCut_ = ps.getDouble("total_outer_cut");
        backEcalCut_ = ps.getDouble("back_ecal_cut");
        ratioCut_ = ps.getDouble("ratio_cut");

        EcalLayerEdepRaw_.resize(nEcalLayers_, 0);
        EcalLayerEdepReadout_.resize(nEcalLayers_, 0);
        EcalLayerOuterRaw_.resize(nEcalLayers_, 0);
        EcalLayerOuterReadout_.resize(nEcalLayers_, 0);
        EcalLayerTime_.resize(nEcalLayers_, 0);
        cellMap_.resize(nEcalLayers_,std::map<int,float>());
        cellMapIso_.resize(nEcalLayers_,std::map<int,float>());


    }

    void EcalVetoProcessor::produce(Event& event) {
    	for (int i = 0; i < nEcalLayers_; i++){
            cellMap_[i].clear();
            cellMapIso_[i].clear();
    	}

    	nReadoutHits_  = 0;
        nIsoHits_      = 0;
        maxIsoDep_     = 0;
        summedIso_     = 0;
        longestMipTrack_ = 0;
        mipTrackDep_   = 0;
        trackVector_.clear();
        std::fill(EcalLayerEdepRaw_.begin(), EcalLayerEdepRaw_.end(), 0);
        std::fill(EcalLayerEdepReadout_.begin(), EcalLayerEdepReadout_.end(), 0);
        std::fill(EcalLayerOuterRaw_.begin(), EcalLayerOuterRaw_.end(), 0);
        std::fill(EcalLayerOuterReadout_.begin(), EcalLayerOuterReadout_.end(), 0);
        std::fill(EcalLayerTime_.begin(), EcalLayerTime_.end(), 0); 

        // Get the collection of digitized Ecal hits from the event. 
        const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
        int nEcalHits = ecalDigis->GetEntriesFast();

        std::cout << "[ EcalVetoProcessor ] : Got " << nEcalHits << " ECal digis in event " << event.getEventHeader()->getEventNumber() << std::endl;

        int globalCentroid = GetShowerCentroidID(ecalDigis);
        fillHitMap(ecalDigis,cellMap_);
        fillIsolatedHitMap(ecalDigis,globalCentroid,cellMap_,cellMapIso_);
        fillMipTracks(globalCentroid,cellMapIso_,trackVector_);

        //Loop over the hits from the event to calculate the rest of the important quantities
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            //Layer-wise quantities
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            EcalLayerEdepRaw_[hit_pair.first] = EcalLayerEdepRaw_[hit_pair.first] + hit->getEnergy();

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
        float summedDet = 0, summedOuter = 0, backSummedDet = 0;

        for (int iLayer = 0; iLayer < EcalLayerEdepReadout_.size(); iLayer++) {
    		for (auto cell : cellMapIso_[iLayer]){
    			if (cell.second > 0){
    				nIsoHits_++;
    				summedIso_ += cell.second;
    			}

    			if (cell.second > maxIsoDep_){
    				maxIsoDep_ = cell.second;
    			}
    		}
    		EcalLayerTime_[iLayer] = EcalLayerTime_[iLayer] / EcalLayerEdepReadout_[iLayer];
            summedDet += EcalLayerEdepReadout_[iLayer];
            summedOuter += EcalLayerOuterReadout_[iLayer];
            if (iLayer > backEcalStartingLayer_)
                backSummedDet += EcalLayerEdepReadout_[iLayer];
        }
        std::cout << "[ EcalVetoProcessor ]:\n" <<
        		"The global centroid = " << globalCentroid << "\n" << std::endl;
        /*std::cout << "[ EcalVetoProcessor ]:\n" 
          << "\t EdepRaw[0] : " << EcalLayerEdepRaw_[0] << "\n"
          << "\t EdepReadout[0] : " << EcalLayerEdepReadout_[0] << "\n"
          << "\t EdepLayerOuterRaw[0] : " << EcalLayerOuterRaw_[0] << "\n"
          << "\t EdepLayerOuterReadout[0] : " << EcalLayerOuterReadout_[0] << "\n"
          << "\t EdepLayerTime[0] : " << EcalLayerTime_[0] << "\n"
          << "\t Shower Median: " << showerMedianCellId 
          << std::endl;*/
        nMipTracks_ = trackVector_.size();

        for (auto track : trackVector_){
        	mipTrackDep_ += track.second;
        	if (track.first > longestMipTrack_){
        		longestMipTrack_ = track.first;
        	}
        }

        result_.setResult(true, globalCentroid, nReadoutHits_, nIsoHits_, nMipTracks_, mipTrackDep_, longestMipTrack_,
        		summedDet, summedOuter,summedIso_, backSummedDet, maxIsoDep_, EcalLayerEdepRaw_);
        event.addToCollection("EcalVeto", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
