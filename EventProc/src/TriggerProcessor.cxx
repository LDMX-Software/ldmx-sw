#include "TString.h"

#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "Event/Event.h"
#include "EventProc/TriggerProcessor.h"
#include "Framework/EventProcessor.h"

void TriggerProcessor::configure(const ldmxsw::ParameterSet& pSet) {
    
    float layerESumCut_ = pSet.getDouble("threshold");
    int mode_ = pSet.getInteger("mode");
    int startLayer_ = pSet.getInteger("start_layer");
    int endLayer_ = pSet.getInteger("end_layer");
}

void TriggerProcessor::produce(event::Event& event) {

    using namespace event;

    /** Grab the Ecal hit collection for the given event */
    const TClonesArray *ecalDigis = event.getCollection("ecalDigis");
    int numEcalHits = ecalDigis->GetEntriesFast();
    
    /** Loop over all ecal hits in the given event */
    for(int iHit = 0; iHit < numEcalHits; ++iHit) {

        EcalHit *hit = (EcalHit*) ecalDigis->At(iHit);
        layerDigiE_[hit->getLayer()] += hit->getEnergy(); 
    }
    
    float layerSum = 0;

    if(mode_ == 0) { // Sum over all cells in a given layer

        TString algoName = "LayerSumTrig";
        std::string algoNametemp = "test";
        for(int iL = startLayer_; iL < endLayer_; ++iL) {
            layerSum += layerDigiE_[iL];
        }

        if(layerSum <= layerESumCut_) {pass_ = true;}

        result_.set(algoName, pass_, 3);

        /** First trigger variable is number of ECAL layers
         *  used in the sum and the second variable is the 
         *  total energy sum from those layers.
         */
        result_.setAlgoVar(0,endLayer_-startLayer_);
        result_.setAlgoVar(1,layerESumCut_);
        result_.setAlgoVar(2,layerSum);
    }

    else if(mode_ == 1) { // Sum over cells in central tower only

        TString algoName = "CenterTower";
        
    }

    event.add(algoNametemp, &result_);

}
