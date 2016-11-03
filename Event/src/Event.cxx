#include "Event/Event.h"

#include "Event/EventConstants.h"

ClassImp(event::Event)

namespace event {

Event::Event()
    : TObject() {
}

Event::~Event() {

    Clear();

    for (CollectionMap::iterator iColl = collMap.begin();
            iColl != collMap.end(); iColl++) {
        delete (*iColl).second;
    }

    collMap.clear();
}

void Event::Clear(Option_t*) {

    TObject::Clear();
    
    eventNumber = -1;
    run = -1;
    timestamp = -1;
    weight = 1.0;

    for (CollectionMap::iterator iColl = collMap.begin();
                iColl != collMap.end(); iColl++) {
        (*iColl).second->Clear("C");
    }
}


}
