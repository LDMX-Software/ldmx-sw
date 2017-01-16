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
#include "Event/EventHeader.h"

// STL
#include <string>
#include <map>

namespace event {

class Event {

    public:

        /**
         * Class constructor.
         */
        Event() {;}

        /**
         * Class destructor.
         */
        virtual ~Event() {;}
        
        virtual const EventHeader* getEventHeader() const = 0;

        template<typename ObjectType> const ObjectType get(std::string name) {
            std::cout << "[ Event ] : Getting object " << name << " with automatic logic pass " << std::endl;
            return (ObjectType) getReal(name, "");
        }

        template<typename ObjectType> const ObjectType get(std::string name, std::string passName) {
            std::cout << "[ Event ] : Getting object " << name << " with pass " << passName << std::endl;
            return (ObjectType) getReal(name, passName);
        }

        virtual void add(const std::string& collectionName, TClonesArray* clones) = 0;
 
        virtual void add(const std::string& collectionName, TObject* obj) = 0;

        const TClonesArray* getCollection(const std::string& collectionName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with automatic logic" << std::endl;
            return (TClonesArray*) getReal(collectionName, "");
        }

        const TClonesArray* getCollection(const std::string& collectionName, std::string passName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with pass " << passName << std::endl;
            return (TClonesArray*) getReal(collectionName, passName);
        }

    protected:
        virtual const TObject* getReal(const std::string& itemName, const std::string& passName) = 0;
  
};

}

#endif
