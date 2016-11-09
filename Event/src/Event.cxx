#include "Event/Event.h"

#include "Event/EventConstants.h"

ClassImp(event::Event)

namespace event {

Event::Event()
    : TObject() {
}

Event::~Event() {

    Clear();

    for (CollectionMap::iterator iColl = collMap_.begin();
            iColl != collMap_.end(); iColl++) {
        delete (*iColl).second;
    }

    collMap_.clear();
}

void Event::Clear(Option_t*) {

    TObject::Clear();
    
    eventNumber_ = -1;
    run_ = -1;
    timestamp_ = -1;
    weight_ = 1.0;

    for (CollectionMap::iterator iColl = collMap_.begin();
                iColl != collMap_.end(); iColl++) {
        (*iColl).second->Clear("C");
    }
}


}
