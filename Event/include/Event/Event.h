#ifndef Event_Event_h
#define Event_Event_h

// ROOT
#include "TObject.h"
#include "TClonesArray.h"

// LDMX
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

// STL
#include <string>
#include <map>

namespace event {

class Event: public TObject {

    public:

        typedef std::map<std::string, TClonesArray*> CollectionMap;

        Event();

        virtual ~Event();

        void Clear(Option_t* = "");

        int getEventNumber();

        int getRun();

        int getTimestamp();
        
        double getWeight();

        void setEventNumber(int);

        void setRun(int);

        void setTimestamp(int);
        
        void setWeight(double);

        TClonesArray* getCollection(const std::string&);

    private:

        int eventNumber;
        int run;
        int timestamp;
        double weight;

        TClonesArray* simParticles;
        TClonesArray* taggerSimHits;
        TClonesArray* recoilSimHits;
        TClonesArray* ecalSimHits;
        TClonesArray* hcalSimHits;

        CollectionMap collectionMap; //!

        ClassDef(Event, 1);
};

}

#endif
