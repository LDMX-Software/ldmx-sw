#include "EventProc/EcalVetoProcessor.h"

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/Event.h"
#include <algorithm>

const int EcalVetoProcessor::NUM_ECAL_LAYERS = 33;
const int EcalVetoProcessor::BACK_ECAL_STARTING_LAYER = 20;
const int EcalVetoProcessor::NUM_LAYERS_FOR_MED_CAL = 10;
const float EcalVetoProcessor::TOTAL_DEP_CUT = 25;
const float EcalVetoProcessor::TOTAL_ISO_CUT = 15;
const float EcalVetoProcessor::BACK_ECAL_CUT = 1;
const float EcalVetoProcessor::RATIO_CUT = 10;

void EcalVetoProcessor::configure(const ldmxsw::ParameterSet&) {
    hexReadout_ = new EcalHexReadout();
}

void EcalVetoProcessor::produce(event::Event& event) {
    using namespace event;

    std::vector<float> EcalLayerEdepRaw(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerEdepReadout(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerIsoRaw(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerIsoReadout(NUM_ECAL_LAYERS, 0);
    std::vector<float> EcalLayerTime(NUM_ECAL_LAYERS, 0);

    const TClonesArray* ecalDigis = event.getCollection("ecalDigis");

    // looper over sim hits
    int numEcalHits = ecalDigis->GetEntriesFast();

    std::cout << "[ EcalVetoProcessor ] : Got " << numEcalHits << " ECal digis in event " << event.getEventHeader()->getEventNumber() << std::endl;

    std::vector<cell_energy_pair> layerMaxCellId(NUM_LAYERS_FOR_MED_CAL, std::make_pair(0, 0));

    //First, we find layer-wise max cell ids
    for (int iHit = 0; iHit < numEcalHits; iHit++) {
        EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
        layer_cell_pair hit_pair = hitToPair(hit);

        if (hit_pair.first < NUM_LAYERS_FOR_MED_CAL) {
            if (layerMaxCellId[hit_pair.first].second < hit->getEnergy()) {
                layerMaxCellId[hit_pair.first] = std::make_pair(hit_pair.second, hit->getEnergy());
            }
        }
    }

    //Sort the layer-wise max energy deposition cells by energy and then select the median
    std::sort(layerMaxCellId.begin(), layerMaxCellId.end(), [](const cell_energy_pair & a, const cell_energy_pair & b)
    {
        return a.second > b.second;
    });
    int showerMedianCellId = layerMaxCellId[layerMaxCellId.size() / 2].first;

    //Loop over the hits from the event to calculate the rest of the important quantities
    for (int iHit = 0; iHit < numEcalHits; iHit++) {
        //Layer-wise quantities
        EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
        layer_cell_pair hit_pair = hitToPair(hit);

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
        if (iLayer > BACK_ECAL_STARTING_LAYER)
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

    doesPassVeto_ = (summedDep < TOTAL_DEP_CUT && summedIso < TOTAL_ISO_CUT && backSummedDep < BACK_ECAL_CUT); // add ratio cut in at some point

    result_.set("EcalVetoV1", doesPassVeto_, 3);
    result_.setAlgoVar(0, summedDep);
    result_.setAlgoVar(0, summedIso);
    result_.setAlgoVar(0, backSummedDep);
    event.add("EcalVeto", &result_);
}

