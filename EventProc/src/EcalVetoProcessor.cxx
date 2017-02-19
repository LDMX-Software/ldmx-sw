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
    totalIsoCut_ = ps.getDouble("total_iso_cut");
    backEcalCut_ = ps.getDouble("back_ecal_cut");
    ratioCut_ = ps.getDouble("ratio_cut");
}

void EcalVetoProcessor::produce(Event& event) {
    
    std::vector<float> EcalLayerEdepRaw(nEcalLayers_, 0);
    std::vector<float> EcalLayerEdepReadout(nEcalLayers_, 0);
    std::vector<float> EcalLayerIsoRaw(nEcalLayers_, 0);
    std::vector<float> EcalLayerIsoReadout(nEcalLayers_, 0);
    std::vector<float> EcalLayerTime(nEcalLayers_, 0);
   
    // Get the collection of digitized Ecal hits from the event. 
    const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
    int nEcalHits = ecalDigis->GetEntriesFast();
    
    //std::cout << "[ EcalVetoProcessor ] : Got " << nEcalHits << " ECal digis in event " << event.getEventHeader()->getEventNumber() << std::endl;

    std::vector<CellEnergyPair> layerMaxCellId(nLayersMedCal_, std::make_pair(0, 0));

    //First, we find layer-wise max cell ids
    for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
        
        // Get the nth digitized Ecal hit
        EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
        LayerCellPair hit_pair = hitToPair(hit);

        if (hit_pair.first < nLayersMedCal_) {
            if (layerMaxCellId[hit_pair.first].second < hit->getEnergy()) {
                layerMaxCellId[hit_pair.first] = std::make_pair(hit_pair.second, hit->getEnergy());
            }
        }
    }

    //Sort the layer-wise max energy deposition cells by energy and then select the median
    std::sort(layerMaxCellId.begin(), layerMaxCellId.end(), [](const CellEnergyPair & a, const CellEnergyPair & b)
    {
        return a.second > b.second;
    });
    int showerMedianCellId = layerMaxCellId[layerMaxCellId.size() / 2].first;

    //Loop over the hits from the event to calculate the rest of the important quantities
    for (int iHit = 0; iHit < nEcalHits; iHit++) {
        //Layer-wise quantities
        EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
        LayerCellPair hit_pair = hitToPair(hit);

        EcalLayerEdepRaw[hit_pair.first] += hit->getEnergy();

        if (hit->getEnergy() > 0) {
            EcalLayerEdepReadout[hit_pair.first] += hit->getEnergy();
            EcalLayerTime[hit_pair.first] += (hit->getEnergy()) * hit->getTime();
        }
        //Check iso
        if (!(hexReadout_->isInShowerInnerRing(showerMedianCellId, hit_pair.second)) && !(hexReadout_->isInShowerOuterRing(showerMedianCellId, hit_pair.second)) && !(hit_pair.second == showerMedianCellId)) {

            EcalLayerIsoRaw[hit_pair.first] += hit->getEnergy();

            if (hit->getEnergy() > 0)
                EcalLayerIsoReadout[hit_pair.first] += hit->getEnergy();
        }
    } // end loop over sim hits
    float summedDep = 0, summedIso = 0, backSummedDep = 0;
    for (int iLayer = 0; iLayer < EcalLayerEdepReadout.size(); iLayer++) {
        EcalLayerTime[iLayer] = EcalLayerTime[iLayer] / EcalLayerEdepReadout[iLayer];
        summedDep += EcalLayerEdepReadout[iLayer];
        summedIso += EcalLayerIsoReadout[iLayer];
        if (iLayer > backEcalStartingLayer_)
            backSummedDep += EcalLayerEdepReadout[iLayer];
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

    doesPassVeto_ = (summedDep < totalDepCut_ && summedIso < totalIsoCut_ && backSummedDep < backEcalCut_); // add ratio cut in at some point
    /*

    result_.set("EcalVetoV1", doesPassVeto_, 3);
    result_.setAlgoVar(0, summedDep);
    result_.setAlgoVar(1, summedIso);
    result_.setAlgoVar(2, backSummedDep);
    event.add("EcalVeto", &result_);
    */
}

}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
