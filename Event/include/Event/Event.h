/**
 * @file Event.h
 * @brief Class defining an interface for accessing event data
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

/**
 * @namespace event
 * @brief Physics event model interfaces and implementation classes
 */
namespace event {

/**
 * @class Event
 * @brief Defines an interface for accessing event data
 *
 * @note
 * A backing EventImpl object provides the actual data collections
 * via ROOT data structures.
 */
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
        
        /**
         * Get the event header.
         * @return The event header.
         */
        EventHeader* getEventHeader() {
            return (EventHeader*) eventImpl_->get("EventHeader", eventImpl_->getPassName());
        }

        /**
         * Get a named object with a specific type, using the default pass name.
         * @return A named object from the event.
         */
        template<typename ObjectType> const ObjectType get(std::string name) {
            return (ObjectType) eventImpl_->get(name, eventImpl_->getPassName());
        }

        /**
         * Get a named object with a specific type and pass name.
         * @param name The name of the object.
         * @param passName The pass name.
         */
        template<typename ObjectType> const ObjectType get(std::string name, std::string passName) {
            return (ObjectType) eventImpl_->get(name, passName);
        }

        /**
         * Add a collection of objects to the event using the default pass name.
         * @param collectionName The name of the collection.
         * @param tca The TClonesArray containing the objects.
         */
        void add(const std::string& collectionName, TClonesArray* tca) {
            eventImpl_->add(collectionName, tca);
        }

        /**
         * Add a named object to the event using the default pass name.
         * @param name The name of the object.
         * @param obj The object to add.
         */
        void add(const std::string& name, TObject* obj) {
            eventImpl_->add(name, obj);
        }

        /**
         * Get a collection of objects by name using the default pass name.
         * @param collectionName The name of the collection.
         */
        const TClonesArray* getCollection(const std::string& collectionName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with pass " << eventImpl_->getPassName() << std::endl;
            return (TClonesArray*) eventImpl_->get(collectionName, eventImpl_->getPassName());
        }

        /**
         * Get a collection of objects by name with the pass name.
         * @param collectionName The name of the collection.
         * @param passName The pass name.
         */
        const TClonesArray* getCollection(const std::string& collectionName, std::string passName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with pass " << passName << std::endl;
            return (TClonesArray*) eventImpl_->get(collectionName, passName);
        }

    private:

        /**
         * The object containing the event data.
         */
        EventImpl* eventImpl_;
};

}

#endif
