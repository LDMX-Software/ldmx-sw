// ROOT
#include "TString.h"

// STL
#include <cmath>

#include "Event/SimCalorimeterHit.h"
#include "EventProc/SimHitSortProcessor.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    void SimHitSortProcessor::configure(const ParameterSet& pSet) {
        collectionName = pSet.getString("simHitCollection");
        outputCollection = pSet.getString("outputCollection");
    }

    void SimHitSortProcessor::produce(Event& event) {

        std::vector<SimCalorimeterHit> ecalSimHits = event.getCollection<SimCalorimeterHit>(collectionName);

        std::sort(ecalSimHits.begin(),ecalSimHits.end(),[](SimCalorimeterHit &a, SimCalorimeterHit &b) {
                return a.getEdep() > b.getEdep();   
            });

        event.add( outputCollection , ecalSimHits );
    }

}

DECLARE_PRODUCER_NS(ldmx, SimHitSortProcessor)
