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
#import "TPython.h"

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

	inline class BDTHelper{

		public:

			BDTHelper(TString importBDTFile, int FeatureVecLen) {
				TPython * pyEnv = new TPython::TPython();
				/* Train a fake bdt, to be replaced by real bdt in future!!! */

				//pyEnv->Exec("import xgboost as xgb");
				pyEnv->Exec("import numpy as np");
				pyEnv->Exec("a = np.random.rand(100," + FeatureVecLen +")");
				pyEnv->Exec("b = np.zeros(len(a))");
				pyEnv->Exec("b[0:50] = 1");
				//pyEnv->Exec("param = {}");
				//pyEnv->Exec("param[\'objective\'] = \'binary:logistic\'");
				//pyEnv->Exec("model = xgb.train(param,xgb.DMatrix(a,label = b))");
			};
			virtual ~BDTHelper() {;}

			double getSinglePred(vector<float> featureVector){
				if (featureVector.size() != nFeatures){
					throw std::runtime_error("Error: You passed " << featureVector.size() << " instead of " << nFeatures);
				}
				TString cmd = vectorToPredCMD(featureVector);
				//pyEnv->Exec("pred = " + cmd);
				//double pred = pyEnv->Eval("pred");
				double pred = rand()%100 * 1/100.;
				return pred;
			};

			TString vectorToPredCMD(vector<float> featureVector){
				TString featuresStrVector = "[[";
				for (int i = 0; i < featureVector.size(); i++){
					featuresStrVector += std::to_string(featureVector[i]);
					if (i < featureVector.size() - 1)
							featuresStrVector += ",";
				}
				featuresStrVector += "]]";
				TString cmd = "float(model.predict(xgb.DMatrix(" + featuresStrVector + "))[0])";
				return cmd;
			};
		private:
			int nFeatures;
	};

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
            inline int GetShowerCentroidID(const TClonesArray* ecalDigis){
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
            		CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
    		        XYCoords centroidCoords = hexReadout_->getCellCentroidXYPair(hit_pair.second);


                    float deltaR = pow( pow((centroidCoords.first - wgtCentroidCoords.first),2)
                            +  pow((centroidCoords.second - wgtCentroidCoords.second),2),.5);
                    if ( deltaR < maxDist ){
                        maxDist = deltaR;
                        returnCellId = hit_pair.second;
                    }
            	}
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
            		std::vector<std::map<int,float>>& cellMap_, std::vector<std::map<int,float>>& cellMapIso_){
            	int nEcalHits = ecalDigis->GetEntriesFast();
				for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
					std::pair<bool,int> isolatedHit = std::make_pair(true,0);
					EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
					LayerCellPair hit_pair = hitToPair(hit);

					//Disregard hits that are on the centroid.
					if(hit_pair.second == globalCentroid) continue;

					//Skip hits that are on centroid inner ring
					if (hexReadout_->isInShowerInnerRing(globalCentroid, hit_pair.second)) {
						continue;
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

            inline void fillMipTracks(int globalCentroid,std::vector<std::map<int,float>>& cellMapIso_, std::vector<std::pair<int,float>>& trackVector){
        		for (int iLayer = 0; iLayer < cellMapIso_.size() - 1; iLayer++){
        			std::vector<LayerCellPair> trackCellPairs;
        			float trackEdep = 0;
        		    for (auto && seedCell : cellMapIso_[iLayer]){
        		    	LayerCellPair seedCellPair = std::make_pair(iLayer,seedCell.first);
        		        trackEdep += seedCell.second;
        		        trackCellPairs.clear();
        		        trackCellPairs.push_back(seedCellPair);
        		        if (globalCentroid == seedCellPair.second || hexReadout_->isInShowerInnerRing(globalCentroid, seedCellPair.second)) {
        		        	continue;
        		        }

        		        while(true){
        		            if (seedCellPair.first + 1 >= cellMapIso_.size() - 1) {
        		            	break;
        		            }
							float matchCellDep;
							LayerCellPair matchCellPair = std::make_pair(-1,1e6);

                            for (auto matchCell : cellMapIso_[seedCellPair.first+1]){
                		        matchCellDep = matchCell.second;
                		        LayerCellPair tempMatchCellPair  = std::make_pair(seedCellPair.first + 1,matchCell.first);
                		        if (globalCentroid == tempMatchCellPair.second ||
                		        		hexReadout_->isInShowerInnerRing(globalCentroid, tempMatchCellPair.second)) continue;
                		        if (tempMatchCellPair.second == seedCellPair.second ||
                		        		hexReadout_->isInShowerInnerRing(tempMatchCellPair.first, seedCellPair.second)){
                		        	matchCellPair = tempMatchCellPair;
                		        	break;
                		        }
                            }

                            if (matchCellPair.first != -1){
                            	trackCellPairs.push_back(matchCellPair);
                            	trackEdep 	 +=  matchCellDep;
                		        seedCellPair  = matchCellPair;
                            }

                            else{
                            	break;
                            }
        		        }

        		        if (trackCellPairs.size() >= 3) {
        		        	trackVector.push_back(std::make_pair(trackCellPairs.size(),trackEdep));
                            for (auto cell : trackCellPairs){
                            	std::map<int,float>::iterator it  = cellMapIso_[cell.first].find(cell.second);
								cellMapIso_[cell.first].erase(it);

                            }
        		        }
        		    }
        		}
            }



        private:

            std::vector<std::map<int,float>> cellMap_;
            std::vector<std::map<int,float>> cellMapIso_;
            std::vector<float> EcalLayerEdepRaw_; 
            std::vector<float> EcalLayerEdepReadout_;
            std::vector<float> EcalLayerOuterRaw_;
            std::vector<float> EcalLayerOuterReadout_;
            std::vector<float> EcalLayerTime_;
            std::vector<float> bdtFeatures;
            std::vector<std::pair<int,float>> trackVector_;

            int nEcalLayers_;
            int nLayersMedCal_; 
            int backEcalStartingLayer_;
            //Begin New variables
            int nReadoutHits_;
            int nIsoHits_;
            int nMipTracks_;
            int longestMipTrack_;
            int nBDTVars;

            std::vector<float> EcalIsoHitsEnergy;
            std::vector<float> EcalMipTrackLength;
            std::vector<float> EcalMipTrackEdep;

            //End New Variables
            double totalDepCut_;
            double totalOuterCut_;
            double backEcalCut_;
            double ratioCut_;
            double summedIso_;
            double maxIsoDep_;
            double bdtCutVal_;
            float mipTrackDep_;
            EcalVetoResult result_;
            EcalDetectorID detID_;
            bool verbose_{false};
            bool doesPassVeto_{false};
            EcalHexReadout* hexReadout_{nullptr};
            BDTHelper* BDTHelper_;
    };

}

#endif
