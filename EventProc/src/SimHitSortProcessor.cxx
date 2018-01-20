// ROOT
#include "TString.h"

// STL
#include <cmath>

#include "Event/SimCalorimeterHit.h"
#include "EventProc/SimHitSortProcessor.h"
#include "Framework/EventProcessor.h"
#include "TClonesArray.h"

namespace ldmx {

    void SimHitSortProcessor::configure(const ParameterSet& pSet) {
        //sortedHits = new TClonesArray(pSet.getString("simHitCollection").c_str(),10000);
        sortedHits = new TClonesArray(EventConstants::SIM_CALORIMETER_HIT.c_str(),10000);
        collectionName = pSet.getString("simHitCollection");
        outputCollection = pSet.getString("outputCollection");
    }

    void SimHitSortProcessor::produce(Event& event) {
        TClonesArray* ecalSimHits = (TClonesArray*) event.getCollection(collectionName);

        int numEcalSimHits = ecalSimHits->GetEntries();
        std::vector<SimCalorimeterHit*> vecSimCaloHit;

        for(int iHit = 0; iHit < numEcalSimHits; ++iHit){
            vecSimCaloHit.push_back((SimCalorimeterHit*)ecalSimHits->At(iHit));
        }// end loop over sim hits

        std::sort(vecSimCaloHit.begin(),vecSimCaloHit.end(),[](SimCalorimeterHit* a, SimCalorimeterHit* b) {
                return a->getEdep() > b->getEdep();   
            });

        for(int iHit = 0 ; iHit < vecSimCaloHit.size() ; iHit++){
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) (sortedHits->ConstructedAt(iHit));
            simHit->setID( vecSimCaloHit[iHit]->getID() );
            simHit->setEdep( vecSimCaloHit[iHit]->getEdep() );
            simHit->setPosition( vecSimCaloHit[iHit]->getPosition()[0] , vecSimCaloHit[iHit]->getPosition()[1] , vecSimCaloHit[iHit]->getPosition()[2] );
            simHit->setTime( vecSimCaloHit[iHit]->getTime() );
        }

        event.add(outputCollection.c_str(),sortedHits);
    }

}

DECLARE_PRODUCER_NS(ldmx, SimHitSortProcessor)
