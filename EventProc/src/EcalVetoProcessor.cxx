#include "EventProc/EcalVetoProcessor.h"

#include "TString.h"
#include "TFile.h"
#include "TClonesArray.h"

#include "Event/SimEvent.h"
#include "EventProc/EventLoop.h"
#include "EventProc/RootEventSource.h"

using event::SimEvent;
using event::SimCalorimeterHit;;
using eventproc::EventLoop;
using eventproc::RootEventSource;

#include "EventProc/EventProcessor.h"

const int eventproc::EcalVetoProcessor::backEcalStartingLayer = 20;
const int eventproc::EcalVetoProcessor::numLayersForMedCal    = 10;
const float eventproc::EcalVetoProcessor::meanNoise           = .15;
const float eventproc::EcalVetoProcessor::readoutThreshold    = 3*meanNoise;

const float eventproc::EcalVetoProcessor::totalDepCut = 25;
const float eventproc::EcalVetoProcessor::totalIsoCut = 15;
const float eventproc::EcalVetoProcessor::backEcalCut = 1;
const float eventproc::EcalVetoProcessor::ratioCut = 10;

eventproc::EcalVetoProcessor::EcalVetoProcessor(TTree* outputTree) 
    : outputTree_{outputTree} { 
}

eventproc::EcalVetoProcessor::~EcalVetoProcessor() { 
}

void eventproc::EcalVetoProcessor::initialize(){

    //////////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - output tree setup - - - - - - - - - - - - -  //
    //////////////////////////////////////////////////////////////////////////
    outputTree_->Branch("ecalLayerEdepRaw",    &ecalLayerEdepRaw_);
    outputTree_->Branch("ecalLayerEdepNoise",  &ecalLayerEdepReadout_);
    outputTree_->Branch("ecalLayerIsoRaw",     &ecalLayerIsoRaw_);
    outputTree_->Branch("ecalLayerIsoReadout", &ecalLayerIsoReadout_);
    outputTree_->Branch("ecalLayerTime",       &ecalLayerTime_);

    /*
     * For now we make four flat arrays containing the ECAL hit information
     *          (to be replaced at a later date w/ custom object)
     */
    outputTree_->Branch("ecalHitId",     &ecalHitId_);
    outputTree_->Branch("ecalHitLayer",  &ecalHitLayer_);
    outputTree_->Branch("ecalHitDep",    &ecalHitDep_);
    outputTree_->Branch("ecalHitNoise",  &ecalHitNoise_);

    outputTree_->Branch("DoesPassVeto", &doesPassVeto);

}

void eventproc::EcalVetoProcessor::execute(){

    // looper over sim hits
    TClonesArray* ecalHits = getEvent()->getCollection(event::EventConstants::ECAL_SIM_HITS);
    int numEcalSimHits = ecalHits->GetEntries();

    std::vector<cell_energy_pair> layerMaxCellId(numLayersForMedCal,std::make_pair(0,0));
    std::vector<float> hitNoise(numEcalSimHits,0);


    //First we simulate noise injection into each hit and store layer-wise max cell ids
    for(int iHit = 0; iHit < numEcalSimHits; iHit++){
        SimCalorimeterHit* ecalHit = (SimCalorimeterHit*) ecalHits->At(iHit);
        hitNoise[iHit] = noiseInjector->Gaus(0,.15);
        layer_cell_pair hit_pair = hitToPair(ecalHit);

        ecalHitId_.push_back(hit_pair.second);
        ecalHitLayer_.push_back(hit_pair.first);
        ecalHitDep_.push_back(ecalHit->getEdep());
        ecalHitNoise_.push_back(hitNoise[iHit]);
        if (hit_pair.first < numLayersForMedCal){
            if (layerMaxCellId[hit_pair.first].second < ecalHit->getEdep() + hitNoise[iHit]){
                layerMaxCellId[hit_pair.first] = std::make_pair(hit_pair.second,ecalHit->getEdep());
            }
        }
    }

    //Sort the layer-wise max energy deposition cells by energy and then select the median
    std::sort (layerMaxCellId.begin(), layerMaxCellId.end(),
            [](const cell_energy_pair & a, const cell_energy_pair & b)
            {
                return a.second > b.second;
            });
    int showerMedianCellId = layerMaxCellId[layerMaxCellId.size()/2].first;

    //Loop over the hits from the event to calculate the rest of the important quantities
    for(int iHit = 0; iHit < numEcalSimHits; iHit++){
        //Layer-wise quantities
        SimCalorimeterHit* ecalHit = (SimCalorimeterHit*) ecalHits->At(iHit);
        layer_cell_pair hit_pair = hitToPair(ecalHit);
        ecalLayerEdepRaw_[hit_pair.first] += ecalHit->getEdep();

        if (ecalHit->getEdep()  + hitNoise[iHit] > readoutThreshold){
            ecalLayerEdepReadout_[hit_pair.first] +=  ecalHit->getEdep()  + hitNoise[iHit];
            ecalLayerTime_[hit_pair.first] +=  (ecalHit->getEdep()  + hitNoise[iHit]) * ecalHit->getTime();
        }
        //Check iso
        if (!(hexReadout->isInShowerInnerRing(showerMedianCellId,hit_pair.second)) &&
            !(hexReadout->isInShowerOuterRing(showerMedianCellId,hit_pair.second)) &&
            !(hit_pair.second == showerMedianCellId)){

            ecalLayerIsoRaw_[hit_pair.first]   +=  ecalHit->getEdep();

            if (ecalHit->getEdep()  + hitNoise[iHit] > readoutThreshold)
                ecalLayerIsoReadout_[hit_pair.first] +=  ecalHit->getEdep()  + hitNoise[iHit];
        }
    }// end loop over sim hits
    float summedDep = 0,summedIso = 0, backSummedDep = 0;
    for (int iLayer=  0; iLayer < ecalLayerEdepReadout_.size(); iLayer++){
        ecalLayerTime_[iLayer] = ecalLayerTime_[iLayer]/ecalLayerEdepReadout_[iLayer];
        summedDep += ecalLayerEdepReadout_[iLayer];
        summedIso += ecalLayerIsoReadout_[iLayer];
        if (iLayer > backEcalStartingLayer) backSummedDep += ecalLayerEdepReadout_[iLayer];
    }

    /*if(verbose){
        std::cout << "EdepRaw[0] : " << ecalLayerEdepRaw_[0] << std::endl;
        std::cout << "EdepReadout[0] : " << ecalLayerEdepReadout_[0] << std::endl;
        std::cout << "EcalLayerIsoRaw[0]: " << ecalLayerIsoRaw_[0] << std::endl;
        std::cout << "EcalLayerIsoReadout[0]: " << ecalLayerIsoReadout_[0] << std::endl;
        std::cout << "EcalLayerTime[0]: " << ecalLayerTime_[0] << std::endl;

        std::cout << "Shower Median : " << showerMedianCellId << std::endl;
    }// end verbose */

    doesPassVeto = (summedDep < totalDepCut && summedIso < totalIsoCut && backSummedDep < backEcalCut && totalDepCut/totalIsoCut < ratioCut);
    outputTree_->Fill();

    std::fill(ecalLayerEdepRaw_.begin(), ecalLayerEdepRaw_.begin(), 0);  
    std::fill(ecalLayerEdepReadout_.begin(), ecalLayerEdepReadout_.end(), 0);  
    std::fill(ecalLayerIsoRaw_.begin(), ecalLayerIsoRaw_.end(), 0);
    std::fill(ecalLayerIsoReadout_.begin(), ecalLayerIsoReadout_.end(), 0);
    std::fill(ecalLayerTime_.begin(), ecalLayerTime_.end(), 0);

    ecalHitId_.clear();
    ecalHitLayer_.clear();
    ecalHitDep_.clear();
    ecalHitNoise_.clear();
}

void eventproc::EcalVetoProcessor::finish(){
}
