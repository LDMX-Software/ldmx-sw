#include "TString.h"

#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "Event/Event.h"
#include "EventProc/TriggerProcessor.h"
#include "Framework/EventProcessor.h"

void TriggerProcessor::configure(const ldmxsw::ParameterSet& pSet) {
    
    layerESumCut_ = pSet.getDouble("threshold");
    mode_ = pSet.getInteger("mode");
    startLayer_ = pSet.getInteger("start_layer");
    endLayer_ = pSet.getInteger("end_layer");
}

void TriggerProcessor::produce(event::Event& event) {

    using namespace event;

    /** Grab the Ecal hit collection for the given event */
    const TClonesArray *ecalDigis = event.getCollection("ecalDigis");
    int numEcalHits = ecalDigis->GetEntriesFast();

    std::vector<double> layerDigiE(100,0.0); // big empty vector..
    
    /** Loop over all ecal hits in the given event */    
    for(int iHit = 0; iHit < numEcalHits; ++iHit) {
        EcalHit *hit = (EcalHit*) ecalDigis->At(iHit);
	if (hit->getLayer()<layerDigiE.size()) // just to be safe...
	    layerDigiE[hit->getLayer()] += hit->getEnergy(); 
    }
    
    float layerSum = 0;

    if(mode_ == 0) { // Sum over all cells in a given layer

	bool pass=false;
	
        TString algoName = "LayerSumTrig";
        std::string algoNametemp = "test";
        for(int iL = startLayer_; iL < endLayer_; ++iL) {
            layerSum += layerDigiE[iL];
        }

        pass = (layerSum <= layerESumCut_);

        result_.set(algoName, pass, 3);

        result_.setAlgoVar(0,layerSum);
        result_.setAlgoVar(1,layerESumCut_);
        result_.setAlgoVar(2,endLayer_-startLayer_);

	event.addToCollection("Trigger",result_);
    }

    else if(mode_ == 1) { // Sum over cells in central tower only

        TString algoName = "CenterTower";
        result_.set(algoName, false, 3);
	
    }


}

DECLARE_PRODUCER(TriggerProcessor)
