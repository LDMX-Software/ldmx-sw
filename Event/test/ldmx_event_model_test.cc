#include "Event/Event.h"

#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"

#include <stdio.h>

int main(int, const char* argv[])  {

    printf("Hello LDMX Event Model Test!\n");

    Event* event = new Event();

    SimCalorimeterHit* calHit = new SimCalorimeterHit();
    SimTrackerHit* trackerHit = new SimTrackerHit();
    SimParticle* particle = new SimParticle();

    delete calHit;
    delete trackerHit;
    delete particle;
    delete event;

    printf("Bye LDMX Event Model Test!\n");
    return 0;
}
