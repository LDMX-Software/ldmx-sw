// ROOT
#include "TString.h"

// STL
#include <cmath>

#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "EventProc/TriggerProcessor.h"
#include "Framework/EventProcessor.h"
#include "DetDescr/EcalHexReadout.h"

namespace ldmx {

    void TriggerProcessor::configure(const ParameterSet& pSet) {

        layerESumCut_ = pSet.getDouble("threshold");
        mode_ = pSet.getInteger("mode");
        startLayer_ = pSet.getInteger("start_layer");
        endLayer_ = pSet.getInteger("end_layer");

        if (mode_ == 0) {
            algoName_ = "LayerSumTrig";
        } else if (mode_ == 1) {
            algoName_ = "CenterTower";
        }
    }

    void TriggerProcessor::produce(Event& event) {

        /** Grab the Ecal hit collection for the given event */
        const TClonesArray *ecalDigis = event.getObject<TClonesArray *>("ecalDigis");
        int numEcalHits = ecalDigis->GetEntriesFast();

        std::vector<double> layerDigiE(100, 0.0); // big empty vector..

        /** Loop over all ecal hits in the given event */
        for (int iHit = 0; iHit < numEcalHits; ++iHit) {
            EcalHit *hit = (EcalHit*) ecalDigis->At(iHit);
            if (hit->getLayer() < layerDigiE.size()) { // just to be safe...
                if (mode_ == 0) { // Sum over all cells in a given layer
                    layerDigiE[hit->getLayer()] += hit->getEnergy();
                } else if (mode_ == 1) { // Sum over cells in central tower only
                    //std::pair<float, float> xyPos = hit->getCellCentroidXYPair(hit->getID());
                    //float cellRadius = sqrt(pow(xyPos.first, 2) + pow(xyPos.second, 2));
                    //if (cellRadius < MAGICNUMBERHERE) {
                    //    layerDigiE[hit->getLayer()] += hit->getEnergy();
                    //}
                }
            }
        }

        float layerSum = 0;
        bool pass = false;

        for (int iL = startLayer_; iL < endLayer_; ++iL) {
            layerSum += layerDigiE[iL];
        }

        pass = (layerSum <= layerESumCut_);

        result_.set(algoName_, pass, 3);
        result_.setAlgoVar(0, layerSum);
        result_.setAlgoVar(1, layerESumCut_);
        result_.setAlgoVar(2, endLayer_ - startLayer_);

        event.add("Trigger", result_);

        // mark the event
        if (pass) 
            setStorageHint(hint_shouldKeep);
        else 
            setStorageHint(hint_shouldDrop);
    }
}

DECLARE_PRODUCER_NS(ldmx, TriggerProcessor)
