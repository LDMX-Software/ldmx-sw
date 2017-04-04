#include "EventProc/EcalVetoProcessor.h"

// ROOT
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TPython.h"

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"

// C++
#include <algorithm>
#include <stdlib.h>
#include <fstream>

namespace ldmx {

    BDTHelper::BDTHelper(TString importBDTFile) {

        // Import the python packages and load the features into xgboost.
        TPython::Exec("print 'Importing BDT python packages'");
        TPython::Exec("print 'importing xgb'; import xgboost as xgb; print xgb");
        TPython::Exec("print 'importing numpy'; import numpy as np; print np");
        TPython::Exec("print 'importing pkl'; import pickle as pkl; print pkl");
        std::cout << "Unpickling bdt from file = " << importBDTFile << std::endl;
        TPython::Exec("print 'importing model in xgb'; model = pkl.load(open('" + importBDTFile + "','r')); print model");
        // ; model.dump_model('model.txt')
    }

    void BDTHelper::buildFeatureVector(std::vector<float>& bdtFeatures, ldmx::EcalVetoResult& result) {
        /*for (int i = 0; i < 33; i++) {
            bdtFeatures.push_back(result.getEcalLayerEdepReadout()[i]);
        }*/
        bdtFeatures.push_back(result.getNReadoutHits());
        bdtFeatures.push_back(result.getNLooseIsoHits());
        bdtFeatures.push_back(result.getNTightIsoHits());
        bdtFeatures.push_back(result.nLooseMipTracks());
        bdtFeatures.push_back(result.nMediumMipTracks());
        bdtFeatures.push_back(result.nTightMipTracks());
        bdtFeatures.push_back(result.getSummedDet());
        bdtFeatures.push_back(result.getSummedOuter());
        bdtFeatures.push_back(result.getBackSummedDep());
        bdtFeatures.push_back(result.getSummedLooseIso());
        bdtFeatures.push_back(result.getMaxLooseIsoDep());
        bdtFeatures.push_back(result.getSummedTightIso());
        bdtFeatures.push_back(result.getMaxTightIsoDep());
        bdtFeatures.push_back(result.getMaxCellDep());
        bdtFeatures.push_back(result.getShowerRMS());
        double maxLen = 0, summedDep = 0;
        for (auto track : result.getLooseMipTracks()) {
            if (track.first > maxLen)
                maxLen = track.first;
            summedDep += track.second;
        }
        bdtFeatures.push_back(maxLen);
        bdtFeatures.push_back(summedDep);

        maxLen = 0, summedDep = 0;
        for (auto track : result.getMediumMipTracks()) {
            if (track.first > maxLen)
                maxLen = track.first;
            summedDep += track.second;
        }
        bdtFeatures.push_back(maxLen);
        bdtFeatures.push_back(summedDep);

        maxLen = 0, summedDep = 0;
        for (auto track : result.getTightMipTracks()) {
            if (track.first > maxLen)
                maxLen = track.first;
            summedDep += track.second;
        }
        bdtFeatures.push_back(maxLen);
        bdtFeatures.push_back(summedDep);
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
        backEcalStartingLayer_ = ps.getInteger("back_ecal_starting_layer");

        bdtCutVal_ = ps.getDouble("disc_cut");
        ecalLayerEdepRaw_.resize(nEcalLayers_, 0);
        ecalLayerEdepReadout_.resize(nEcalLayers_, 0);
        ecalLayerOuterRaw_.resize(nEcalLayers_, 0);
        ecalLayerOuterReadout_.resize(nEcalLayers_, 0);
        ecalLayerTime_.resize(nEcalLayers_, 0);
        cellMap_.resize(nEcalLayers_, std::map<int, float>());
        cellMapLooseIso_.resize(nEcalLayers_, std::map<int, float>());
        cellMapTightIso_.resize(nEcalLayers_, std::map<int, float>());
    }

    void EcalVetoProcessor::produce(Event& event) {
        for (int i = 0; i < nEcalLayers_; i++) {
            cellMap_[i].clear();
            cellMapLooseIso_[i].clear();
            cellMapTightIso_[i].clear();
        }

        // FIXME: These commands should go above the corresponding variables.
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

        nReadoutHits_ = 0;
        nLooseIsoHits_ = 0;
        nTightIsoHits_ = 0;
        summedDet_ = 0;
        summedOuter_ = 0;
        backSummedDet_ = 0;
        summedLooseIso_ = 0;
        maxLooseIsoDep_ = 0;
        summedTightIso_ = 0;
        maxTightIsoDep_ = 0;
        maxCellDep_ = 0;
        showerRMS_ = 0;

        std::fill(ecalLayerEdepRaw_.begin(), ecalLayerEdepRaw_.end(), 0);
        std::fill(ecalLayerEdepReadout_.begin(), ecalLayerEdepReadout_.end(), 0);
        std::fill(ecalLayerOuterRaw_.begin(), ecalLayerOuterRaw_.end(), 0);
        std::fill(ecalLayerOuterReadout_.begin(), ecalLayerOuterReadout_.end(), 0);
        std::fill(ecalLayerTime_.begin(), ecalLayerTime_.end(), 0);

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
        fillIsolatedHitMap(ecalDigis, globalCentroid, cellMap_, cellMapLooseIso_, !doTight);
        int trackLen = 2;

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
            }
            //Check Outer
            if (!(hexReadout_->isInShowerInnerRing(globalCentroid, hit_pair.second))
                    && !(hexReadout_->isInShowerOuterRing(globalCentroid, hit_pair.second))
                    && !(hit_pair.second == globalCentroid)) {

                ecalLayerOuterRaw_[hit_pair.first] += hit->getEnergy();

                if (hit->getEnergy() > 0)
                    ecalLayerOuterReadout_[hit_pair.first] += hit->getEnergy();
            }
        }

        // end loop over sim hits

        for (int iLayer = 0; iLayer < ecalLayerEdepReadout_.size(); iLayer++) {
            for (auto cell : cellMapLooseIso_[iLayer]) {
                if (cell.second > 0) {
                    nLooseIsoHits_++;
                    summedLooseIso_ += cell.second;
                }

                if (cell.second > maxLooseIsoDep_) {
                    maxLooseIsoDep_ = cell.second;
                }
            }
            for (auto cell : cellMapTightIso_[iLayer]) {
                if (cell.second > 0) {
                    nTightIsoHits_++;
                    summedTightIso_ += cell.second;
                }

                if (cell.second > maxTightIsoDep_) {
                    maxTightIsoDep_ = cell.second;
                }
            }
            ecalLayerTime_[iLayer] = ecalLayerTime_[iLayer] / ecalLayerEdepReadout_[iLayer];
            summedDet_ += ecalLayerEdepReadout_[iLayer];
            summedOuter_ += ecalLayerOuterReadout_[iLayer];
            if (iLayer > backEcalStartingLayer_)
                backSummedDet_ += ecalLayerEdepReadout_[iLayer];
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
        std::vector<std::map<int, float>> looseIsoCopy(cellMapLooseIso_);
        fillMipTracks(cellMapLooseIso_, looseMipTracks_, trackLen);
        trackLen = 3;
        fillMipTracks(looseIsoCopy, mediumMipTracks_, trackLen);
        fillMipTracks(cellMapTightIso_, tightMipTracks_, trackLen);

        result_.setVariables(nReadoutHits_, nLooseIsoHits_, nTightIsoHits_, summedDet_, summedOuter_, backSummedDet_,
                summedLooseIso_, maxLooseIsoDep_, summedTightIso_, maxTightIsoDep_, maxCellDep_, showerRMS_,
                ecalLayerEdepReadout_, looseMipTracks_, mediumMipTracks_, tightMipTracks_);
        if (doBdt_) {
            BDTHelper_->buildFeatureVector(bdtFeatures_, result_);
            float pred = BDTHelper_->getSinglePred(bdtFeatures_);
            result_.setVetoResult(pred > bdtCutVal_);
            result_.setDiscValue(pred);
            std::cout << "  pred > bdtCutVal = " << (pred > bdtCutVal_) << std::endl;
        }
        event.addToCollection("EcalVeto", result_);
    }

    EcalVetoProcessor::LayerCellPair EcalVetoProcessor::hitToPair(EcalHit* hit) {
        int detIDraw = hit->getID();
        detID_.setRawValue(detIDraw);
        detID_.unpack();
        int layer = detID_.getFieldValue("layer");
        int cellid = detID_.getFieldValue("cell");
        return (std::make_pair(layer, cellid));
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
            XYCoords centroidCoords = hexReadout_->getCellCentroidXYPair(hit_pair.second);
            wgtCentroidCoords.first = wgtCentroidCoords.first + centroidCoords.first * cell_energy_pair.second;
            wgtCentroidCoords.second = wgtCentroidCoords.second + centroidCoords.second * cell_energy_pair.second;
            sumEdep += cell_energy_pair.second;
        }
        wgtCentroidCoords.first = wgtCentroidCoords.first / sumEdep;
        wgtCentroidCoords.second = wgtCentroidCoords.second / sumEdep;
        //Find Nearest Cell to Centroid
        float maxDist = 1e6;
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);
            XYCoords centroidCoords = hexReadout_->getCellCentroidXYPair(hit_pair.second);

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
                if (hexReadout_->isInShowerInnerRing(globalCentroid, hit_pair.second)) {
                    continue;
                }
            }

            //Skip hits that have a readout neighbor
            std::vector<int> cellNbrIds = hexReadout_->getInnerRingCellIds(hit_pair.second);

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

    void EcalVetoProcessor::fillMipTracks(std::vector<std::map<int, float>>& cellMapIso_, std::vector<std::pair<int, float>>& trackVector, int minTrackLen) {

        std::vector<std::vector<int>> trackTuple;
        for (int iLayer = 0; iLayer < cellMapIso_.size() - 1; iLayer++) {
            float trackEdep = 0;
            //for (auto && seedCell : cellMapIso_[iLayer]){
            auto itEnd = cellMapIso_[iLayer].cend();
            for (auto it = cellMapIso_[iLayer].cbegin(); it != cellMapIso_[iLayer].cend();) {

                auto seedCell = (*it);              //.second;

                std::pair<int, int> trackEndPoints;
                std::vector<LayerCellPair> trackCellPairs;
                LayerCellPair seedCellPair = std::make_pair(iLayer, seedCell.first);
                trackEdep += seedCell.second;
                trackCellPairs.clear();
                trackCellPairs.push_back(seedCellPair);
                trackEndPoints.first = seedCell.first;

                while (true) {
                    if (seedCellPair.first + 1 >= cellMapIso_.size() - 1) {
                        break;
                    }
                    float matchCellDep;
                    LayerCellPair matchCellPair = std::make_pair(-1, 1e6);
                    for (auto matchCell : cellMapIso_[seedCellPair.first + 1]) {
                        matchCellDep = matchCell.second;
                        LayerCellPair tempMatchCellPair = std::make_pair(seedCellPair.first + 1, matchCell.first);
                        if (tempMatchCellPair.second == seedCellPair.second || hexReadout_->isInShowerInnerRing(tempMatchCellPair.first, seedCellPair.second)) {
                            matchCellPair = tempMatchCellPair;
                            break;
                        }
                    }

                    if (matchCellPair.first != -1) {
                        trackCellPairs.push_back(matchCellPair);
                        trackEdep += matchCellDep;
                        seedCellPair = matchCellPair;
                        trackEndPoints.second = matchCellPair.first;
                    }

                    else {
                        break;
                    }
                }

                if (trackCellPairs.size() >= minTrackLen) {
                    trackVector.push_back(std::make_pair(trackCellPairs.size(), trackEdep));
                    std::vector<int> trackInfo = {iLayer, trackEndPoints.first, trackEndPoints.second};
                    trackTuple.push_back(trackInfo);
                    int counter = 0;
                    cellMapIso_[iLayer].erase(it++);
                    for (auto cell : trackCellPairs) {
                        if (counter == 0) {
                            counter = counter + 1;
                            continue;
                        }
                        std::map<int, float>::iterator it_2 = cellMapIso_[cell.first].find(cell.second);
                        cellMapIso_[cell.first].erase(it_2);

                    }
                } else {
                    it++;
                }
            }
        }

        for (int iTrack = 0; iTrack < trackVector.size(); iTrack++) {
            if (iTrack >= trackVector.size() - 1)
                break;
            int prevEndLayer = trackTuple[iTrack][0];
            int prevEndId = trackTuple[iTrack][2];
            int prevLen = trackVector[iTrack].first;
            for (int jTrack = 0; jTrack < trackVector.size(); jTrack++) {
                if (jTrack >= trackVector.size() - 1)
                    break;
                int nextStartLayer = trackTuple[jTrack][0];
                int nextStartId = trackTuple[jTrack][1];
                if (prevEndLayer + prevLen != nextStartLayer - 1)
                    break;
                if (hexReadout_->isInShowerOuterRing(prevEndId, nextStartId)) {
                    trackVector[iTrack].second = trackVector[iTrack].second + trackVector[jTrack].second;
                    trackVector[iTrack].first = trackVector[iTrack].first + trackVector[jTrack].first;
                    trackTuple[iTrack][2] = trackTuple[jTrack][2];

                    trackVector.erase(trackVector.begin() + jTrack);
                    trackTuple.erase(trackTuple.begin() + jTrack);
                    jTrack--;
                }
            }
        }
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
