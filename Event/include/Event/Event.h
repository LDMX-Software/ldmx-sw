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

        /**
         * Class constructor.
         */
        Event(EventImpl* eventImpl) : eventImpl_(eventImpl) {;}

        /**
         * Class destructor.
         */
        virtual ~Event() {;}
        
        EventHeader* getEventHeader() {
            std::cout << "[ Event ] : Getting EventHeader with pass " << eventImpl_->getPassName() << std::endl;
            return (EventHeader*) eventImpl_->get("EventHeader", eventImpl_->getPassName());
        }

        template<typename ObjectType> const ObjectType get(std::string name) {
            std::cout << "[ Event ] : Getting object " << name << " with pass " << eventImpl_->getPassName() << std::endl;
            return (ObjectType) eventImpl_->get(name, eventImpl_->getPassName());
        }

        template<typename ObjectType> const ObjectType get(std::string name, std::string passName) {
            std::cout << "[ Event ] : Getting object " << name << " with pass " << passName << std::endl;
            return (ObjectType) eventImpl_->get(name, passName);
        }

        void add(const std::string& collectionName, TClonesArray* clones) {
            eventImpl_->add(collectionName, clones);
        }

        void add(const std::string& collectionName, TObject* obj) {
            eventImpl_->add(collectionName, obj);
        }

        const TClonesArray* getCollection(const std::string& collectionName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with pass " << eventImpl_->getPassName() << std::endl;
            return (TClonesArray*) eventImpl_->get(collectionName, eventImpl_->getPassName());
        }

        const TClonesArray* getCollection(const std::string& collectionName, std::string passName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with pass " << passName << std::endl;
            return (TClonesArray*) eventImpl_->get(collectionName, passName);
        }

    private:

        EventImpl* eventImpl_;
};

}

#endif
