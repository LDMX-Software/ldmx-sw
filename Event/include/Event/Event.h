/**
 * @file Event.h
 * @brief Class defining an abstract interface for accessing event information and data collections
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_EVENT_H_
#define EVENT_EVENT_H_

// ROOT
#include "TObject.h"
#include "TClonesArray.h"

// LDMX
#include "Event/EventConstants.h"
#include "Event/EventHeader.h"
#include "Event/EventImpl.h"
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

        /**
         * Class constructor.
         */
        Event(EventImpl* eventImpl, std::string passName) : eventImpl_(eventImpl), passName_(passName) {;}

        /**
         * Class destructor.
         */
        virtual ~Event() {;}
        
        EventHeader* getEventHeader() {
            return (EventHeader*) eventImpl_->get("EventHeader", passName_);
        }

        template<typename ObjectType> const ObjectType get(std::string name) {
            return (ObjectType) eventImpl_->get(name, passName_);
        }

        template<typename ObjectType> const ObjectType get(std::string name, std::string passName) {
            return (ObjectType) eventImpl_->get(name, passName);
        }

        void add(const std::string& collectionName, TClonesArray* clones) {
            eventImpl_->add(collectionName, clones);
        }

        void add(const std::string& collectionName, TObject* obj) {
            eventImpl_->add(collectionName, obj);
        }

        const TClonesArray* getCollection(const std::string& collectionName) {
            return (TClonesArray*) eventImpl_->get(collectionName, passName_);
        }

        const TClonesArray* getCollection(const std::string& collectionName, std::string passName) {
            return (TClonesArray*) eventImpl_->get(collectionName, passName);
        }

    private:

        EventImpl* eventImpl_;
        std::string passName_;
};

}

#endif
