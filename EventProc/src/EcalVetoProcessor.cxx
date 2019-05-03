#include "EventProc/EcalVetoProcessor.h"

// ROOT
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TPython.h"

// LDMX
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"

// C++
#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <cmath>

namespace ldmx {

    BDTHelper::BDTHelper(TString importBDTFile) {

        // Import the python packages and load the features into xgboost.
        TPython::Exec("print 'Loading xgboost BDT'"); 
        TPython::Exec("import xgboost as xgb");
        TPython::Exec("import numpy as np");
        TPython::Exec("import pickle as pkl");
        TPython::Exec("model = pkl.load(open('" + importBDTFile + "','r'))");
    }

    void BDTHelper::buildFeatureVector(std::vector<float>& bdtFeatures, ldmx::EcalVetoResult& result) {
        bdtFeatures.push_back(result.getNReadoutHits());
        bdtFeatures.push_back(result.getSummedDet());
        bdtFeatures.push_back(result.getSummedTightIso());
        bdtFeatures.push_back(result.getMaxCellDep());
        bdtFeatures.push_back(result.getShowerRMS());
        bdtFeatures.push_back(result.getXStd());
        bdtFeatures.push_back(result.getYStd());
        bdtFeatures.push_back(result.getAvgLayerHit());
        bdtFeatures.push_back(result.getDeepestLayerHit());
        bdtFeatures.push_back(result.getStdLayerHit());
    }
    float BDTHelper::getSinglePred(std::vector<float> bdtFeatures) {
        TString cmd = vectorToPredCMD(bdtFeatures);
        TPython::Exec("pred = " + cmd);
        float pred = TPython::Eval("pred");
        //std::cout << "  pred = " << pred << std::endl;

        return pred;
    }

    TString BDTHelper::vectorToPredCMD(std::vector<float> bdtFeatures) {
        TString featuresStrVector = "[[";
        for (int i = 0; i < bdtFeatures.size(); i++) {
            featuresStrVector += std::to_string(bdtFeatures[i]);
            if (i < bdtFeatures.size() - 1)
                featuresStrVector += ",";
        }
        featuresStrVector += "]]";
        TString cmd = "float(model.predict(xgb.DMatrix(np.array(" + featuresStrVector + ")))[0])";

        return cmd;
    }

    void EcalVetoProcessor::configure(const ParameterSet& ps) {
        doBdt_ = ps.getInteger("do_bdt");
        if (doBdt_){
            // Config and init the BDT.
            bdtFileName_ = ps.getString("bdt_file");
            if (!std::ifstream(bdtFileName_).good()) {
                EXCEPTION_RAISE("EcalVetoProcessor",
                        "The specified BDT file '" + bdtFileName_ + "' does not exist!");
            }

            BDTHelper_ = new BDTHelper(bdtFileName_);
        }

        cellFileNamexy_ = ps.getString("cellxy_file");
        if (!std::ifstream(cellFileNamexy_).good()) {
            EXCEPTION_RAISE("NonFidEcalVetoProcessor",
                            "The specified x,y cell file '" + cellFileNamexy_ + "' does not exist!");
        } else {
            std::ifstream cellxyfile(cellFileNamexy_);
            float valuex;
            float valuey;
            while ( cellxyfile >> valuex >> valuey) {
                mapsx.push_back(valuex);
                mapsy.push_back(valuey);
            }
        }


        hexReadout_ = new EcalHexReadout();
        nEcalLayers_ = ps.getInteger("num_ecal_layers");

        bdtCutVal_ = ps.getDouble("disc_cut");
        ecalLayerEdepRaw_.resize(nEcalLayers_, 0);
        ecalLayerEdepReadout_.resize(nEcalLayers_, 0);
        ecalLayerTime_.resize(nEcalLayers_, 0);
        cellMap_.resize(nEcalLayers_, std::map<int, float>());
        cellMapTightIso_.resize(nEcalLayers_, std::map<int, float>());

        // Set the collection name as defined in the configuration
        collectionName_ = ps.getString("collection_name"); 
    }

    void EcalVetoProcessor::clearProcessor(){
        for (int i = 0; i < nEcalLayers_; i++) {
            cellMap_[i].clear();
            cellMapTightIso_[i].clear();
        }
        bdtFeatures_.clear();

        nReadoutHits_ = 0;
        summedDet_ = 0;
        summedTightIso_ = 0;
        maxCellDep_ = 0;
        showerRMS_ = 0;
        xStd_ = 0;
        yStd_ = 0;
        avgLayerHit_ = 0;
        stdLayerHit_ = 0;
        deepestLayerHit_ = 0;

        std::fill(ecalLayerEdepRaw_.begin(), ecalLayerEdepRaw_.end(), 0);
        std::fill(ecalLayerEdepReadout_.begin(), ecalLayerEdepReadout_.end(), 0);
        std::fill(ecalLayerTime_.begin(), ecalLayerTime_.end(), 0);
    }

    void EcalVetoProcessor::produce(Event& event) {
        result_.Clear();
        clearProcessor();

        // Get the collection of digitized Ecal hits from the event. 
        const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
        int nEcalHits = ecalDigis->GetEntriesFast();

        //std::cout << "[ EcalVetoProcessor ] : Got " << nEcalHits << " ECal digis in event "
        //        << event.getEventHeader()->getEventNumber() << std::endl;

        int globalCentroid = GetShowerCentroidIDAndRMS(ecalDigis, showerRMS_);
        /* ~~ Fill the hit map ~~ O(n)  */
        fillHitMap(ecalDigis, cellMap_);
        bool doTight = true;
        /* ~~ Fill the isolated hit maps ~~ O(n)  */
        fillIsolatedHitMap(ecalDigis, globalCentroid, cellMap_, cellMapTightIso_, doTight);

        //Loop over the hits from the event to calculate the rest of the important quantities

        float wavgLayerHit = 0;
        float xMean = 0;
        float yMean = 0;
        
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            //Layer-wise quantities
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            ecalLayerEdepRaw_[hit_pair.first] = ecalLayerEdepRaw_[hit_pair.first] + hit->getEnergy();
            if (maxCellDep_ < hit->getEnergy())
                maxCellDep_ = hit->getEnergy();
            if (hit->getEnergy() > 0) {
                nReadoutHits_++;
                ecalLayerEdepReadout_[hit_pair.first] += hit->getEnergy();
                ecalLayerTime_[hit_pair.first] += (hit->getEnergy()) * hit->getTime();
                xMean += getCellCentroidXYPair(hit_pair.second).first * hit->getEnergy();
                yMean += getCellCentroidXYPair(hit_pair.second).second * hit->getEnergy();
                avgLayerHit_ += hit->getLayer();
                wavgLayerHit += hit->getLayer() * hit->getEnergy();
                if (deepestLayerHit_ < hit->getLayer()) {
                    deepestLayerHit_ = hit->getLayer();
                }
            }
        }
        
        for (int iLayer = 0; iLayer < ecalLayerEdepReadout_.size(); iLayer++) {
            for (auto cell : cellMapTightIso_[iLayer]) {
                if (cell.second > 0) {
                    summedTightIso_ += cell.second;
                }
            }
            ecalLayerTime_[iLayer] = ecalLayerTime_[iLayer] / ecalLayerEdepReadout_[iLayer];
            summedDet_ += ecalLayerEdepReadout_[iLayer];
        }
        
        if (nReadoutHits_ > 0) {
            avgLayerHit_ /= nReadoutHits_;
            wavgLayerHit /= summedDet_;
            xMean /= summedDet_;
            yMean /= summedDet_;
        } else {
            wavgLayerHit = 0;
            avgLayerHit_ = 0;
            xMean = 0;
            yMean = 0;
        }

        // Loop over hits a second time to find the standard deviations.
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            if (hit->getEnergy() > 0) {
                xStd_ += pow((getCellCentroidXYPair(hit_pair.second).first - xMean), 2) * hit->getEnergy();
                yStd_ += pow((getCellCentroidXYPair(hit_pair.second).second - yMean), 2) * hit->getEnergy();
                stdLayerHit_ += pow((hit->getLayer() - wavgLayerHit), 2) * hit->getEnergy();
            }
        }
        
        if (nReadoutHits_ > 0) {
            xStd_ = sqrt (xStd_ / summedDet_);
            yStd_ = sqrt (yStd_ / summedDet_);
            stdLayerHit_ = sqrt (stdLayerHit_ / summedDet_);
        } else {
            xStd_ = 0;
            yStd_ = 0;
            stdLayerHit_ = 0;
        }
        
        // end loop over sim hits

        // Get the collection of Ecal scoring plane hits. If it doesn't exist,
        // don't bother adding any truth tracking information.

        std::vector<double> recoilP;
        std::vector<float> recoilPos;

        if (event.exists("EcalScoringPlaneHits")) {
            const TClonesArray* ecalSpHits{event.getCollection("EcalScoringPlaneHits")};

            // Loop through all of the sim particles and find the recoil 
            // electron.
            const TClonesArray* simParticles{event.getCollection("SimParticles")};
            SimParticle* recoilElectron{nullptr}; 
            for (int simParticleIndex = 0; simParticleIndex < simParticles->GetEntriesFast();
                    ++simParticleIndex) { 
                SimParticle* particle = static_cast<SimParticle*>(simParticles->At(simParticleIndex)); 

                // We only care about the recoil electron
                if ((particle->getPdgID() == 11) && (particle->getParentCount() == 0)) { 
                    recoilElectron = particle;
                    break;
                } 
            }

            for (int ecalSpIndex = 0; ecalSpIndex < ecalSpHits->GetEntriesFast(); ++ecalSpIndex) {
                SimTrackerHit* spHit =  static_cast<SimTrackerHit*>(ecalSpHits->At(ecalSpIndex)); 
                
                if (spHit->getLayerID() != 1) continue;
                
                SimParticle* spParticle = spHit->getSimParticle();
                if (spParticle == recoilElectron) { 
                    recoilP = spHit->getMomentum();
                    recoilPos = spHit->getPosition();
                    if (recoilP[2] <= 0) continue; 
                    break;
                } 
            }
        }

        /* Code for fiducial region below */
        
        std::vector<float> faceXY(2);
        
        if (!recoilP.empty() && recoilP[2] != 0) {
            faceXY[0] = ((223.8 - 220.0) * (recoilP[0] / recoilP[2])) + recoilPos[0];
            faceXY[1] = ((223.8 - 220.0) * (recoilP[1] / recoilP[2])) + recoilPos[1];
        } else {
            faceXY[0] = -9999.0;
            faceXY[1] = -9999.0;
        }
        
        int inside = 0;
        int up = 0;
        int step = 0;
        int index;
        float cell_radius = 5.0;
        
        std::vector<float>::iterator it;
        it = std::lower_bound(mapsx.begin(), mapsx.end(), faceXY[0]);
        
        index = std::distance( mapsx.begin(), it);
        
        if (index == mapsx.size()) {
            index += -1;
        }
        
        if (!recoilP.empty() && faceXY[0] != -9999.0) {
            while (true) {
                std::vector<double> dis(2);
                
                dis[0] = faceXY[0] - mapsx[index + step];
                dis[1] = faceXY[1] - mapsy[index + step];
                
                float celldis = sqrt (pow(dis[0],2) + pow(dis[1],2));
                
                if (celldis <= cell_radius) {
                    inside = 1;
                    break;
                }
                
                if ((abs(dis[0]) > 5 && up == 0) || index + step == mapsx.size()-1) {
                    up = 1;
                    step = 0;
                } else if ((abs(dis[0]) > 5 && up == 1) || (index + step == 0 && up == 1)) {
                    break;
                }
                
                if (up == 0) {
                    step += 1;
                } else {
                    step += -1;
                }
            }
        }

        result_.setVariables(nReadoutHits_, deepestLayerHit_, summedDet_, summedTightIso_, maxCellDep_,
            showerRMS_, xStd_, yStd_, avgLayerHit_, stdLayerHit_, ecalLayerEdepReadout_, recoilP, recoilPos);
        
        if (doBdt_) {
            BDTHelper_->buildFeatureVector(bdtFeatures_, result_);
            float pred = BDTHelper_->getSinglePred(bdtFeatures_);
            result_.setVetoResult(pred > bdtCutVal_);
            result_.setDiscValue(pred);
            //std::cout << "  pred > bdtCutVal = " << (pred > bdtCutVal_) << std::endl;
        
            // If the event passes the veto, keep it. Otherwise, 
            // drop the event.
            if (result_.passesVeto() && inside) { 
                setStorageHint(hint_shouldKeep); 
            } else { 
                setStorageHint(hint_shouldDrop);
            }
        }
        
        if (inside) {
            setStorageHint(hint_shouldKeep);
        } else {
            setStorageHint(hint_shouldDrop);
        }
        event.addToCollection(collectionName_, result_);
    }

    EcalVetoProcessor::LayerCellPair EcalVetoProcessor::hitToPair(EcalHit* hit) {
        int detIDraw = hit->getID();
        detID_.setRawValue(detIDraw);
        detID_.unpack();
        int layer = detID_.getFieldValue("layer");
        int cellid = detID_.getFieldValue("cell");
        int moduleid = detID_.getFieldValue("module_position");
        int combinedid = cellid*10+moduleid;
        return (std::make_pair(layer, combinedid));
    }

    /* Function to calculate the energy weighted shower centroid */
    int EcalVetoProcessor::GetShowerCentroidIDAndRMS(const TClonesArray* ecalDigis, double& showerRMS) {
        int nEcalHits = ecalDigis->GetEntriesFast();
        XYCoords wgtCentroidCoords = std::make_pair<float, float>(0., 0.);
        float sumEdep = 0;
        int returnCellId = 1e6;
        //Calculate Energy Weighted Centroid
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);
            CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
            XYCoords centroidCoords = getCellCentroidXYPair(hit_pair.second);
            wgtCentroidCoords.first = wgtCentroidCoords.first + centroidCoords.first * cell_energy_pair.second;
            wgtCentroidCoords.second = wgtCentroidCoords.second + centroidCoords.second * cell_energy_pair.second;
            sumEdep += cell_energy_pair.second;
        }
        wgtCentroidCoords.first = (sumEdep > 1E-6) ? wgtCentroidCoords.first / sumEdep : wgtCentroidCoords.first;
        wgtCentroidCoords.second = (sumEdep > 1E-6) ? wgtCentroidCoords.second / sumEdep : wgtCentroidCoords.second;
        //Find Nearest Cell to Centroid
        float maxDist = 1e6;
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);
            XYCoords centroidCoords = getCellCentroidXYPair(hit_pair.second);

            float deltaR = pow(pow((centroidCoords.first - wgtCentroidCoords.first), 2) + pow((centroidCoords.second - wgtCentroidCoords.second), 2), .5);
            showerRMS += deltaR * hit->getEnergy();
            if (deltaR < maxDist) {
                maxDist = deltaR;
                returnCellId = hit_pair.second;
            }
        }
        if (sumEdep > 0)
            showerRMS = showerRMS / sumEdep;
        return returnCellId;
    }

                /* Function to load up empty vector of hit maps */
    void EcalVetoProcessor::fillHitMap(const TClonesArray* ecalDigis,
            std::vector<std::map<int, float>>& cellMap_) {
        int nEcalHits = ecalDigis->GetEntriesFast();
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(
                    hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);

            CellEnergyPair cell_energy_pair = std::make_pair(
                    hit_pair.second, hit->getEnergy());
            cellMap_[hit_pair.first].insert(cell_energy_pair);
        }
    }

    void EcalVetoProcessor::fillIsolatedHitMap(const TClonesArray* ecalDigis, float globalCentroid,
            std::vector<std::map<int, float>>& cellMap_, std::vector<std::map<int, float>>& cellMapIso_, bool doTight) {
        int nEcalHits = ecalDigis->GetEntriesFast();
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            std::pair<bool, int> isolatedHit = std::make_pair(true, 0);
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);
            if (doTight) {
                //Disregard hits that are on the centroid.
                if (hit_pair.second == globalCentroid)
                    continue;

                //Skip hits that are on centroid inner ring
                if (isInShowerInnerRing(globalCentroid, hit_pair.second)) {
                    continue;
                }
            }

            //Skip hits that have a readout neighbor
            std::vector<int> cellNbrIds = getInnerRingCellIds(hit_pair.second);

            //Get neighboring cell id's and try to look them up in the full cell map (constant speed algo.)
            for (int k = 0; k < 6; k++) {
                std::map<int, float>::iterator it = cellMap_[hit_pair.first].find(cellNbrIds[k]);
                if (it != cellMap_[hit_pair.first].end()) {
                    isolatedHit = std::make_pair(false, cellNbrIds[k]);
                    break;
                }
            }
            if (!isolatedHit.first) {
                continue;
            }
            //Insert isolated hit
            CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
            cellMapIso_[hit_pair.first].insert(cell_energy_pair);
        }
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
