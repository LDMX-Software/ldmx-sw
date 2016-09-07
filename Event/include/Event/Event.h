#ifndef EVENT_EVENT_H_
#define EVENT_EVENT_H_ 1

// ROOT
#include "TObject.h"

// LDMX
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

class Event : public TObject {

public:

    Event();
    virtual ~Event();

    ClassDef(Event, 1)

};

#endif
