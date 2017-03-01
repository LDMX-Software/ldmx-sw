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

        EcalLayerEdepRaw_.resize(nEcalLayers_, 0);
        EcalLayerEdepReadout_.resize(nEcalLayers_, 0);
        EcalLayerIsoRaw_.resize(nEcalLayers_, 0);
        EcalLayerIsoReadout_.resize(nEcalLayers_, 0);
        EcalLayerTime_.resize(nEcalLayers_, 0);

    }

    void EcalVetoProcessor::produce(Event& event) {

        std::fill(EcalLayerEdepRaw_.begin(), EcalLayerEdepRaw_.end(), 0);
        std::fill(EcalLayerEdepReadout_.begin(), EcalLayerEdepReadout_.end(), 0);
        std::fill(EcalLayerIsoRaw_.begin(), EcalLayerIsoRaw_.end(), 0);
        std::fill(EcalLayerIsoReadout_.begin(), EcalLayerIsoReadout_.end(), 0);
        std::fill(EcalLayerTime_.begin(), EcalLayerTime_.end(), 0); 

        // Get the collection of digitized Ecal hits from the event. 
        const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
        int nEcalHits = ecalDigis->GetEntriesFast();

        std::cout << "[ EcalVetoProcessor ] : Got " << nEcalHits << " ECal digis in event " << event.getEventHeader()->getEventNumber() << std::endl;

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

        std::sort(layerMaxCellId.begin(), layerMaxCellId.end(), [](const CellEnergyPair & a, const CellEnergyPair & b)
                {
                return a.second > b.second;
                });
        int showerMedianCellId = layerMaxCellId[layerMaxCellId.size() / 2].first;

        //Sort the layer-wise max energy deposition cells by energy and then select the median

        //Loop over the hits from the event to calculate the rest of the important quantities
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            //Layer-wise quantities
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            EcalLayerEdepRaw_[hit_pair.first] = EcalLayerEdepRaw_[hit_pair.first] + hit->getEnergy();

            if (hit->getEnergy() > 0) {
                EcalLayerEdepReadout_[hit_pair.first] += hit->getEnergy();
                EcalLayerTime_[hit_pair.first] += (hit->getEnergy()) * hit->getTime();
            }
            //Check iso
            if (!(hexReadout_->isInShowerInnerRing(showerMedianCellId, hit_pair.second)) && !(hexReadout_->isInShowerOuterRing(showerMedianCellId, hit_pair.second)) && !(hit_pair.second == showerMedianCellId)) {

                EcalLayerIsoRaw_[hit_pair.first] += hit->getEnergy();

                if (hit->getEnergy() > 0)
                    EcalLayerIsoReadout_[hit_pair.first] += hit->getEnergy();
            }
        }

        // end loop over sim hits
        float summedDep = 0, summedIso = 0, backSummedDep = 0;
        for (int iLayer = 0; iLayer < EcalLayerEdepReadout_.size(); iLayer++) {
            EcalLayerTime_[iLayer] = EcalLayerTime_[iLayer] / EcalLayerEdepReadout_[iLayer];
            summedDep += EcalLayerEdepReadout_[iLayer];
            summedIso += EcalLayerIsoReadout_[iLayer];
            if (iLayer > backEcalStartingLayer_)
                backSummedDep += EcalLayerEdepReadout_[iLayer];
        }


        /*std::cout << "[ EcalVetoProcessor ]:\n" 
          << "\t EdepRaw[0] : " << EcalLayerEdepRaw_[0] << "\n"
          << "\t EdepReadout[0] : " << EcalLayerEdepReadout_[0] << "\n"
          << "\t EdepLayerIsoRaw[0] : " << EcalLayerIsoRaw_[0] << "\n"
          << "\t EdepLayerIsoReadout[0] : " << EcalLayerIsoReadout_[0] << "\n" 
          << "\t EdepLayerTime[0] : " << EcalLayerTime_[0] << "\n"
          << "\t Shower Median: " << showerMedianCellId 
          << std::endl;*/

        doesPassVeto_ = (summedDep < totalDepCut_ && summedIso < totalIsoCut_ && backSummedDep < backEcalCut_); // add ratio cut in at some point
       
        result_.setResult(doesPassVeto_, summedDep, summedIso, backSummedDep);
        event.addToCollection("EcalVeto", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
