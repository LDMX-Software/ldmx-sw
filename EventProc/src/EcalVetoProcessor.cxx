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

void EcalVetoProcessor::configure(const ParameterSet& ps) {
    hexReadout_ = new EcalHexReadout();

    NUM_ECAL_LAYERS = ps.getInteger("num_ecal_layers");
    BACK_ECAL_STARTING_LAYER = ps.getInteger("back_ecal_starting_layers");
    NUM_LAYERS_FOR_MED_CAL = ps.getInteger("num_layers_for_med_cal");
    TOTAL_DEP_CUT = ps.getDouble("total_dep_cut");
    TOTAL_ISO_CUT = ps.getDouble("total_iso_cut");
    BACK_ECAL_CUT = ps.getDouble("back_ecal_cut");
    RATIO_CUT = ps.getDouble("ratio_cut");
}

void EcalVetoProcessor::produce(Event& event) {
    std::vector<float> EcalLayerEdepRaw(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerEdepReadout(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerIsoRaw(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerIsoReadout(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerTime(NUM_ECAL_LAYERS, 0);
    
    const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
    
    // looper over sim hits
    int numEcalHits = ecalDigis->GetEntriesFast();
    
    std::cout << "[ EcalVetoProcessor ] : Got " << numEcalHits << " ECal digis in event " << event.getEventHeader()->getEventNumber() << std::endl;

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

    /*
     if(verbose){
     std::cout << "EdepRaw[0] : " << (*EcalLayerEdepRaw_)[0] << std::endl;
     std::cout << "EdepReadout[0] : " << (*EcalLayerEdepReadout_)[0] << std::endl;
     std::cout << "EcalLayerIsoRaw[0]: " << (*EcalLayerIsoRaw_)[0] << std::endl;
     std::cout << "EcalLayerIsoReadout[0]: " << (*EcalLayerIsoReadout_)[0] << std::endl;
     std::cout << "EcalLayerTime[0]: " << (*EcalLayerTime_)[0] << std::endl;
     
     std::cout << "Shower Median : " << showerMedianCellId << std::endl;
     }// end verbose
     */

    doesPassVeto_ = (summedDep < TOTAL_DEP_CUT && summedIso < TOTAL_ISO_CUT && backSummedDep < BACK_ECAL_CUT); // add ratio cut in at some point
    /*

    result_.set("EcalVetoV1", doesPassVeto_, 3);
    result_.setAlgoVar(0, summedDep);
    result_.setAlgoVar(1, summedIso);
    result_.setAlgoVar(2, backSummedDep);
    event.add("EcalVeto", &result_);
    */
}

void eventproc::EcalVetoProcessor::finish(){
}
