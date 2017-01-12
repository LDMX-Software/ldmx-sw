#include "EventProc/EcalVetoProcessor.h"

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/SimEvent.h"
#include "EventProc/EventLoop.h"
#include "EventProc/RootEventSource.h"

using event::SimEvent;
using event::SimCalorimeterHit;;
using eventproc::EventLoop;
using eventproc::RootEventSource;

#include "EventProc/EventProcessor.h"

const int eventproc::EcalVetoProcessor::numEcalLayers         = 33;
const int eventproc::EcalVetoProcessor::backEcalStartingLayer = 20;
const int eventproc::EcalVetoProcessor::numLayersForMedCal    = 10;
const float eventproc::EcalVetoProcessor::meanNoise           = .015;
const float eventproc::EcalVetoProcessor::readoutThreshold    = 3*meanNoise;

const float eventproc::EcalVetoProcessor::totalDepCut = 25;
const float eventproc::EcalVetoProcessor::totalIsoCut = 15;
const float eventproc::EcalVetoProcessor::backEcalCut = 1;
const float eventproc::EcalVetoProcessor::ratioCut = 10;

void eventproc::EcalVetoProcessor::initialize(){
    detID = new EcalDetectorID();
    hexReadout = new EcalHexReadout();
    noiseInjector = new TRandom2(0);

    //////////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - output tree setup - - - - - - - - - - - - -  //
    //////////////////////////////////////////////////////////////////////////

    EcalLayerEdepRaw_     = new std::vector<float>(numEcalLayers,0);
    EcalLayerEdepReadout_ = new std::vector<float>(numEcalLayers,0);
    EcalLayerIsoRaw_      = new std::vector<float>(numEcalLayers,0);
    EcalLayerIsoReadout_  = new std::vector<float>(numEcalLayers,0);
    EcalLayerTime_        = new std::vector<float>(numEcalLayers,0);
    EcalHitId_       = new std::vector<float>();
    EcalHitLayer_    = new std::vector<float>();
    EcalHitDep_      = new std::vector<float>();

    //this->getEvent()->add("EcalLayerEdepRaw", &(EcalLayerEdepRaw_));
    /*


    outputTree->Branch("EcalLayerEdepRaw",&(EcalLayerEdepRaw_));
    outputTree->Branch("EcalLayerEdepNoise",&(EcalLayerEdepReadout_));
    outputTree->Branch("EcalLayerIsoRaw",&(EcalLayerIsoRaw_));
    outputTree->Branch("EcalLayerIsoReadout",&(EcalLayerIsoReadout_));
    outputTree->Branch("EcalLayerTime",&(EcalLayerTime_));*/

    /*
     * For now we make four flat arrays containing the ECAL hit information
     *          (to be replaced at a later date w/ custom object)
     */
    /*outputTree->Branch("EcalHitId",&(EcalHitId_));
    outputTree->Branch("EcalHitLayer",&(EcalHitLayer_));
    outputTree->Branch("EcalHitDep",&(EcalHitDep_));
    outputTree->Branch("EcalHitNoise",&(EcalHitNoise_));

    outputTree->Branch("DoesPassVeto",&(doesPassVeto));*/

}

void eventproc::EcalVetoProcessor::execute(){

    // looper over sim hits
    TClonesArray* EcalHits = (TClonesArray*) getEvent()->get(event::EventConstants::ECAL_SIM_HITS, "recon");
    int numEcalSimHits = EcalHits->GetEntries();

    std::vector<cell_energy_pair> layerMaxCellId(numLayersForMedCal,std::make_pair(0,0));
    std::vector<float> hitNoise(numEcalSimHits,0);


    //First we simulate noise injection into each hit and store layer-wise max cell ids
    for(int iHit = 0; iHit < numEcalSimHits; iHit++){
        SimCalorimeterHit* EcalHit = (SimCalorimeterHit*) EcalHits->At(iHit);
        hitNoise[iHit] = noiseInjector->Gaus(0,meanNoise);
        layer_cell_pair hit_pair = hitToPair(EcalHit);

        EcalHitId_->push_back(hit_pair.second);
        EcalHitLayer_->push_back(hit_pair.first);
        EcalHitDep_->push_back(EcalHit->getEdep());
        EcalHitNoise_->push_back(hitNoise[iHit]);
        if (hit_pair.first < numLayersForMedCal){
            if (layerMaxCellId[hit_pair.first].second < EcalHit->getEdep() + hitNoise[iHit]){
                layerMaxCellId[hit_pair.first] = std::make_pair(hit_pair.second,EcalHit->getEdep());
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
        SimCalorimeterHit* EcalHit = (SimCalorimeterHit*) EcalHits->At(iHit);
        layer_cell_pair hit_pair = hitToPair(EcalHit);

        (*EcalLayerEdepRaw_)[hit_pair.first] += EcalHit->getEdep();

        if (EcalHit->getEdep()  + hitNoise[iHit] > readoutThreshold){
            (*EcalLayerEdepReadout_)[hit_pair.first] +=  EcalHit->getEdep()  + hitNoise[iHit];
            (*EcalLayerTime_)[hit_pair.first] +=  (EcalHit->getEdep()  + hitNoise[iHit]) * EcalHit->getTime();
        }
        //Check iso
        if (!(hexReadout->isInShowerInnerRing(showerMedianCellId,hit_pair.second)) &&
            !(hexReadout->isInShowerOuterRing(showerMedianCellId,hit_pair.second)) &&
            !(hit_pair.second == showerMedianCellId)){

            (*EcalLayerIsoRaw_)[hit_pair.first]   +=  EcalHit->getEdep();

            if (EcalHit->getEdep()  + hitNoise[iHit] > readoutThreshold)
                (*EcalLayerIsoReadout_)[hit_pair.first] +=  EcalHit->getEdep()  + hitNoise[iHit];
        }
    }// end loop over sim hits
    float summedDep = 0,summedIso = 0, backSummedDep = 0;
    for (int iLayer=  0; iLayer < EcalLayerEdepReadout_->size(); iLayer++){
        (*EcalLayerTime_)[iLayer] = (*EcalLayerTime_)[iLayer]/(*EcalLayerEdepReadout_)[iLayer];
        summedDep += (*EcalLayerEdepReadout_)[iLayer];
        summedIso += (*EcalLayerIsoReadout_)[iLayer];
        if (iLayer > backEcalStartingLayer) backSummedDep += (*EcalLayerEdepReadout_)[iLayer];
    }

    if(verbose){
        std::cout << "EdepRaw[0] : " << (*EcalLayerEdepRaw_)[0] << std::endl;
        std::cout << "EdepReadout[0] : " << (*EcalLayerEdepReadout_)[0] << std::endl;
        std::cout << "EcalLayerIsoRaw[0]: " << (*EcalLayerIsoRaw_)[0] << std::endl;
        std::cout << "EcalLayerIsoReadout[0]: " << (*EcalLayerIsoReadout_)[0] << std::endl;
        std::cout << "EcalLayerTime[0]: " << (*EcalLayerTime_)[0] << std::endl;

        std::cout << "Shower Median : " << showerMedianCellId << std::endl;
    }// end verbose

    doesPassVeto = (summedDep < totalDepCut && summedIso < totalIsoCut && backSummedDep < backEcalCut); // add ratio cut in at some point
    //outputTree->Fill();

    EcalLayerEdepRaw_     = new std::vector<float>(numEcalLayers,0);
    EcalLayerEdepReadout_ = new std::vector<float>(numEcalLayers,0);
    EcalLayerIsoRaw_      = new std::vector<float>(numEcalLayers,0);
    EcalLayerIsoReadout_  = new std::vector<float>(numEcalLayers,0);
    EcalLayerTime_        = new std::vector<float>(numEcalLayers,0);

    EcalHitId_       = new std::vector<float>();
    EcalHitLayer_    = new std::vector<float>();
    EcalHitDep_      = new std::vector<float>();
    EcalHitNoise_    = new std::vector<float>();
}

void eventproc::EcalVetoProcessor::finish(){
    delete EcalLayerEdepRaw_;
    delete EcalLayerEdepReadout_;
    delete EcalLayerIsoRaw_;
    delete EcalLayerIsoReadout_;
    delete EcalLayerTime_;
}
