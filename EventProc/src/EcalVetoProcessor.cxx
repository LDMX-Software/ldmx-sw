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
        // FIXME: These debug prints can be removed.
        TPython::Exec("print 'Importing BDT python packages'");
        TPython::Exec("print 'importing xgb'; import xgboost as xgb; print xgb");
        TPython::Exec("print 'importing numpy'; import numpy as np; print np");
        TPython::Exec("print 'importing pkl'; import pickle as pkl; print pkl");
        std::cout << "Unpickling bdt from file = " << importBDTFile << std::endl;
        TPython::Exec("print 'importing model in xgb'; model = pkl.load(open('" + importBDTFile + "','r')); print model");
        // ; model.dump_model('model.txt')
    }

    void BDTHelper::buildFeatureVector(std::vector<float>& bdtFeatures, ldmx::EcalVetoResult& result) {
        bdtFeatures.push_back(result.getNReadoutHits());
        bdtFeatures.push_back(result.getSummedDet());
        bdtFeatures.push_back(result.getSummedTightIso());
        bdtFeatures.push_back(result.getMaxCellDep());
        bdtFeatures.push_back(result.getShowerRMS());
        bdtFeatures.push_back(result.getXStd());
        bdtFeatures.push_back(result.getYStd());
        bdtFeatures.push_back(result.getXMean());
        bdtFeatures.push_back(result.getYMean());
        bdtFeatures.push_back(result.getAvgLayerHit());
        bdtFeatures.push_back(result.getDeepestLayerHit());
        bdtFeatures.push_back(result.getStdLayerHit());
    }
    float BDTHelper::getSinglePred(std::vector<float> bdtFeatures) {
        TString cmd = vectorToPredCMD(bdtFeatures);
        TPython::Exec("pred = " + cmd);
        float pred = TPython::Eval("pred");
        std::cout << "  pred = " << pred << std::endl;

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
            bdtFileName_ = ps.getString("bdt_file", "bdt.pkl");
            if (!std::ifstream(bdtFileName_).good()) {
                EXCEPTION_RAISE("EcalVetoProcessor",
                        "The specified BDT file '" + bdtFileName_ + "' does not exist!");
            }

            BDTHelper_ = new BDTHelper(bdtFileName_);
        }
        hexReadout_ = new EcalHexReadout();
        nEcalLayers_ = ps.getInteger("num_ecal_layers");

        bdtCutVal_ = ps.getDouble("disc_cut");
        ecalLayerEdepRaw_.resize(nEcalLayers_, 0);
        ecalLayerEdepReadout_.resize(nEcalLayers_, 0);
        ecalLayerTime_.resize(nEcalLayers_, 0);
        cellMap_.resize(nEcalLayers_, std::map<int, float>());
        cellMapTightIso_.resize(nEcalLayers_, std::map<int, float>());
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
        xMean_ = 0;
        yMean_ = 0;
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

        std::cout << "[ EcalVetoProcessor ] : Got " << nEcalHits << " ECal digis in event "
                << event.getEventHeader()->getEventNumber() << std::endl;

        int globalCentroid = GetShowerCentroidIDAndRMS(ecalDigis, showerRMS_);
        /* ~~ Fill the hit map ~~ O(n)  */
        fillHitMap(ecalDigis, cellMap_);
        bool doTight = true;
        /* ~~ Fill the isolated hit maps ~~ O(n)  */
        fillIsolatedHitMap(ecalDigis, globalCentroid, cellMap_, cellMapTightIso_, doTight);

        //Loop over the hits from the event to calculate the rest of the important quantities
        
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
                xMean_ += getCellCentroidXYPair(hit_pair.second).first;
                yMean_ += getCellCentroidXYPair(hit_pair.second).second;
                avgLayerHit_ += hit->getLayer();
                if (deepestLayerHit_ < hit->getLayer()) {
                    deepestLayerHit_ = hit->getLayer();
                }
            }
        }
        
        if (nReadoutHits_ > 0) {
        avgLayerHit_ /= nReadoutHits_;
        xMean_ /= nReadoutHits_;
        yMean_ /= nReadoutHits_;
        } else {
        avgLayerHit_ = 0;
        xMean_ = 0;
        yMean_ = 0;
        }
        // Loop over hits a second time to find the standard deviations.
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            if (hit->getEnergy() > 0) {
                xStd_ += pow((getCellCentroidXYPair(hit_pair.second).first - xMean_), 2);
                yStd_ += pow((getCellCentroidXYPair(hit_pair.second).second - yMean_), 2);
                stdLayerHit_ += pow((hit->getLayer() - avgLayerHit_), 2);
            }
        }
        
        if (nReadoutHits_ > 0) {
        xStd_ = sqrt (xStd_ / nReadoutHits_);
        yStd_ = sqrt (yStd_ / nReadoutHits_);
        stdLayerHit_ = sqrt (stdLayerHit_ / nReadoutHits_);
        } else {
        xStd_ = 0;
        yStd_ = 0;
        stdLayerHit_ = 0;
        }
        
        // end loop over sim hits

        for (int iLayer = 0; iLayer < ecalLayerEdepReadout_.size(); iLayer++) {
            for (auto cell : cellMapTightIso_[iLayer]) {
                if (cell.second > 0) {
                    summedTightIso_ += cell.second;
                }
            }
            ecalLayerTime_[iLayer] = ecalLayerTime_[iLayer] / ecalLayerEdepReadout_[iLayer];
            summedDet_ += ecalLayerEdepReadout_[iLayer];
        }

        /*std::cout << "[ EcalVetoProcessor ]:\n" 
         << "\t EdepRaw[0] : " << EcalLayerEdepRaw_[0] << "\n"
         << "\t EdepReadout[0] : " << EcalLayerEdepReadout_[0] << "\n"
         << "\t EdepLayerOuterRaw[0] : " << EcalLayerOuterRaw_[0] << "\n"
         << "\t EdepLayerOuterReadout[0] : " << EcalLayerOuterReadout_[0] << "\n"
         << "\t EdepLayerTime[0] : " << EcalLayerTime_[0] << "\n"
         << "\t Shower Median: " << showerMedianCellId
         << std::endl;*/


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
                    /*std::cout << "[ EcalVetoProcessor ]: " 
                              << "Recoil momentum: [ " 
                              << recoilP[0] 
                              << ", " << recoilP[1]  
                              << ", " << recoilP[2] << " ]" << std::endl;*/
                    break;
                } 
            }
        }

        result_.setVariables(nReadoutHits_, deepestLayerHit_, summedDet_, summedTightIso_, maxCellDep_,
            showerRMS_, xStd_, yStd_, xMean_, yMean_, avgLayerHit_, stdLayerHit_, ecalLayerEdepReadout_, recoilP, recoilPos);
        
        if (doBdt_) {
            BDTHelper_->buildFeatureVector(bdtFeatures_, result_);
            float pred = BDTHelper_->getSinglePred(bdtFeatures_);
            result_.setVetoResult(pred > bdtCutVal_);
            result_.setDiscValue(pred);
            std::cout << "  pred > bdtCutVal = " << (pred > bdtCutVal_) << std::endl;
        
            // If the event passes the veto, keep it. Otherwise, 
            // drop the event.
            if (result_.passesVeto()) { 
                setStorageHint(hint_shouldKeep); 
            } else { 
                setStorageHint(hint_shouldDrop);
            }
        }
        event.addToCollection("EcalVeto", result_);
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
