#ifndef EVENT_EVENT_H_
#define EVENT_EVENT_H_ 1

// ROOT
#include "TObject.h"
#include "TClonesArray.h"

// LDMX
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

// STL
#include <string>

class Event: public TObject {

    public:

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

        int getCollectionSize(const std::string&);

        TObject* addObject(const std::string&);

    public:

        static std::string SIM_PARTICLES;
        static std::string RECOIL_SIM_HITS;
        static std::string TAGGER_SIM_HITS;
        static std::string ECAL_SIM_HITS;
        static std::string HCAL_SIM_HITS;

    private:

        int nextCollectionIndex(const std::string& collectionName);

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

        int nSimParticles;
        int nTaggerSimHits;
        int nRecoilSimHits;
        int nEcalSimHits;
        int nHcalSimHits;

        static int DEFAULT_COLLECTION_SIZE;

        ClassDef(Event, 1);
};

#endif
