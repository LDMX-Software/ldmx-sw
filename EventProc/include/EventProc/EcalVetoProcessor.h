/**
 * @file EcalVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information
 * @author Owen Colegrove, UCSB
 */

#ifndef EVENTPROC_ECALVETOPROCESSOR_H_
#define EVENTPROC_ECALVETOPROCESSOR_H_

// ROOT
#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom2.h"
#include "TClonesArray.h"

// LDMX
#include "Event/EcalVetoResult.h"
#include "Event/EcalHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

//C++
#include <map>

namespace ldmx {

    /**
     * @class EcalVetoProcessor
     * @brief Determines if event is vetoable using ECAL hit information
     */
    class EcalVetoProcessor : public Producer {

        public:

            typedef std::pair<int, int> LayerCellPair;

            typedef std::pair<int, float> CellEnergyPair;

            typedef std::pair<float, float> XYCoords;

            EcalVetoProcessor(const std::string& name,  Process& process) :
                Producer(name, process) {
            }

            virtual ~EcalVetoProcessor() {;}

            void configure(const ParameterSet&);

            void produce(Event& event);

        private:

            inline LayerCellPair hitToPair(EcalHit* hit) {
                int detIDraw = hit->getID();
                detID_.setRawValue(detIDraw);
                detID_.unpack();
                int layer = detID_.getFieldValue("layer");
                int cellid = detID_.getFieldValue("cell");
                return (std::make_pair(layer, cellid));
            }

            /* Function to calculate the energy weighted shower centroid */
            inline int GetShowerCentroidIDAndRMS(const TClonesArray* ecalDigis, double & showerRMS){
            	int nEcalHits = ecalDigis->GetEntriesFast();
            	XYCoords wgtCentroidCoords = std::make_pair<float,float>(0.,0.);
            	float sumEdep = 0;
            	int returnCellId = 1e6;
            	//Calculate Energy Weighted Centroid
            	for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            		EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            		LayerCellPair hit_pair = hitToPair(hit);
            		CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
    		        XYCoords centroidCoords = hexReadout_->getCellCentroidXYPair(hit_pair.second);
    		        wgtCentroidCoords.first   = wgtCentroidCoords.first + centroidCoords.first*cell_energy_pair.second;
    		        wgtCentroidCoords.second  = wgtCentroidCoords.second + centroidCoords.second*cell_energy_pair.second;
    		        sumEdep += cell_energy_pair.second;
    		    }
            	wgtCentroidCoords.first = wgtCentroidCoords.first/sumEdep;
            	wgtCentroidCoords.second = wgtCentroidCoords.second/sumEdep;
            	//Find Nearest Cell to Centroid
            	float maxDist = 1e6;
            	for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            		EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            		LayerCellPair hit_pair = hitToPair(hit);
    		        XYCoords centroidCoords = hexReadout_->getCellCentroidXYPair(hit_pair.second);


                    float deltaR = pow( pow((centroidCoords.first - wgtCentroidCoords.first),2)
                            +  pow((centroidCoords.second - wgtCentroidCoords.second),2),.5);
                    showerRMS += deltaR * hit->getEnergy();
                    if ( deltaR < maxDist ){
                        maxDist = deltaR;
                        returnCellId = hit_pair.second;
                    }
            	}
            	if (sumEdep > 0)
            		showerRMS = showerRMS/sumEdep;
            	return returnCellId;
            }

            /* Function to load up empty vector of hit maps */
            inline void fillHitMap(const TClonesArray* ecalDigis, std::vector<std::map<int,float>>& cellMap_){
            	int nEcalHits = ecalDigis->GetEntriesFast();
				for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
					EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
					LayerCellPair hit_pair = hitToPair(hit);

					CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
					cellMap_[hit_pair.first].insert(cell_energy_pair);
				}
            }

            /* Function to take loaded hit maps and find isolated hits in them */
            inline void fillIsolatedHitMap(const TClonesArray* ecalDigis,float globalCentroid,
            		std::vector<std::map<int,float>>& cellMap_, std::vector<std::map<int,float>>& cellMapIso_, bool doTight = false){
            	int nEcalHits = ecalDigis->GetEntriesFast();
				for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
					std::pair<bool,int> isolatedHit = std::make_pair(true,0);
					EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
					LayerCellPair hit_pair = hitToPair(hit);
					if (doTight){
						//Disregard hits that are on the centroid.
						if(hit_pair.second == globalCentroid) continue;

						//Skip hits that are on centroid inner ring
						if (hexReadout_->isInShowerInnerRing(globalCentroid, hit_pair.second)) {
							continue;
						}
					}

					//Skip hits that have a readout neighbor
					std::vector<int> cellNbrIds = hexReadout_->getInnerRingCellIds(hit_pair.second);

					//Get neighboring cell id's and try to look them up in the full cell map (constant speed algo.)
				    for (int k = 0; k < 6; k++){
				        std::map<int,float>::iterator it = cellMap_[hit_pair.first ].find(cellNbrIds[k]);
				        if(it != cellMap_[hit_pair.first ].end()) {isolatedHit = std::make_pair(false,cellNbrIds[k]); break;}
				    }
				    if (!isolatedHit.first) {
				    	continue;
				    }
				    //Insert isolated hit
					CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
					cellMapIso_[hit_pair.first].insert(cell_energy_pair);
				}
            }

            inline void fillMipTracks(std::vector<std::map<int,float>>& cellMapIso_,
            		std::vector<std::pair<int,float>>& trackVector, int minTrackLen = 2){

            	std::vector<std::vector<int>> trackTuple;
            	for (int iLayer = 0; iLayer < cellMapIso_.size() - 1; iLayer++){
        			float trackEdep = 0;
        		    //for (auto && seedCell : cellMapIso_[iLayer]){
        			auto itEnd = cellMapIso_[iLayer].cend();
        			for (auto it = cellMapIso_[iLayer].cbegin(); it != cellMapIso_[iLayer].cend(); ){

        				auto seedCell = (*it);//.second;

        		    	std::pair<int,int> trackEndPoints;
        		    	std::vector<LayerCellPair> trackCellPairs;
        		    	LayerCellPair seedCellPair = std::make_pair(iLayer,seedCell.first);
        		        trackEdep += seedCell.second;
        		        trackCellPairs.clear();
        		        trackCellPairs.push_back(seedCellPair);
        		        trackEndPoints.first = seedCell.first;

        		        while(true){
        		            if (seedCellPair.first + 1 >= cellMapIso_.size() - 1) {
        		            	break;
        		            }
							float matchCellDep;
							LayerCellPair matchCellPair = std::make_pair(-1,1e6);
                            for (auto matchCell : cellMapIso_[seedCellPair.first+1]){
                		        matchCellDep = matchCell.second;
                		        LayerCellPair tempMatchCellPair  = std::make_pair(seedCellPair.first + 1,matchCell.first);
                		        if (tempMatchCellPair.second == seedCellPair.second ||
                		        		hexReadout_->isInShowerInnerRing(tempMatchCellPair.first, seedCellPair.second)){
                		        	matchCellPair = tempMatchCellPair;
                		        	break;
                		        }
                            }

                            if (matchCellPair.first != -1){
                            	trackCellPairs.push_back(matchCellPair);
                            	trackEdep 	  +=  matchCellDep;
                		        seedCellPair  = matchCellPair;
                		        trackEndPoints.second = matchCellPair.first;
                            }

                            else{
                            	break;
                            }
        		        }

        		        if (trackCellPairs.size() >= minTrackLen) {
        		        	trackVector.push_back(std::make_pair(trackCellPairs.size(),trackEdep));
        		        	std::vector<int> trackInfo = {iLayer,trackEndPoints.first,trackEndPoints.second};
        		        	trackTuple.push_back(trackInfo);
        		        	int counter = 0;
        		        	cellMapIso_[iLayer].erase(it++);
                            for (auto cell : trackCellPairs){
                            	if (counter == 0){
                            		counter = counter + 1;
                            		continue;
                            	}
                            	std::map<int,float>::iterator it_2  = cellMapIso_[cell.first].find(cell.second);
								cellMapIso_[cell.first].erase(it_2);

                            }
        		        }
        		        else{
        		        	it++;
        		        }
        		    }
        		}

    		    for (int iTrack = 0; iTrack < trackVector.size(); iTrack++){
    		    	if (iTrack >= trackVector.size() - 1) break;
    		    	int prevEndLayer = trackTuple[iTrack][0];
    		    	int prevEndId 	 = trackTuple[iTrack][2];
    		    	int prevLen = trackVector[iTrack].first;
    		    	for (int jTrack = 0; jTrack < trackVector.size(); jTrack++){
    		    		if (jTrack >= trackVector.size() - 1) break;
    		    		int nextStartLayer = trackTuple[jTrack][0];
    		    		int nextStartId    = trackTuple[jTrack][1];
    		    		if ( prevEndLayer + prevLen != nextStartLayer - 1 ) break;
    		    		if (hexReadout_->isInShowerOuterRing(prevEndId, nextStartId)){
    		    			trackVector[iTrack].second = trackVector[iTrack].second  + trackVector[jTrack].second;
    		    			trackVector[iTrack].first  = trackVector[iTrack].first + trackVector[jTrack].first;
    		    			trackTuple[iTrack][2] = trackTuple[jTrack][2];

    		    			trackVector.erase (trackVector.begin()+jTrack);
    		    			trackTuple.erase (trackTuple.begin()+jTrack);
    		    			jTrack --;
    		    		}
    		    	}
    		    }
            }
        private:

            std::vector<std::map<int,float>> cellMap_;
            std::vector<std::map<int,float>> cellMapLooseIso_;
            std::vector<std::map<int,float>> cellMapTightIso_;

            std::vector<float> EcalLayerEdepRaw_; 
            std::vector<float> EcalLayerEdepReadout_;
            std::vector<float> EcalLayerOuterRaw_;
            std::vector<float> EcalLayerOuterReadout_;
            std::vector<float> EcalLayerTime_;

            std::vector<std::pair<int,float>> looseMipTracks_;
            std::vector<std::pair<int,float>> mediumMipTracks_;
            std::vector<std::pair<int,float>> tightMipTracks_;

            int nEcalLayers_;
            int backEcalStartingLayer_;
            //Begin New variables
            int nReadoutHits_;
            int nLooseIsoHits_;
    		int nTightIsoHits_;

    		double summedDet_;
    		double summedOuter_;
    		double backSummedDet_;
            double summedLooseIso_;
            double maxLooseIsoDep_;
            double summedTightIso_;
            double maxTightIsoDep_;
            double maxCellDep_;
            double showerRMS_;


            EcalVetoResult result_;
            EcalDetectorID detID_;
            bool verbose_{false};
            bool doesPassVeto_{false};
            EcalHexReadout* hexReadout_{nullptr};
    };

}

#endif
