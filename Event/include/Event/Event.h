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

        bool exists(const std::string& name) {
            return getReal(name, "", false)!=0;
        }

        bool exists(const std::string& name, const std::string& passName) {
            return getReal(name, passName, false)!=0;
        }

        template<typename ObjectType> const ObjectType get(const std::string& name) {
            std::cout << "[ Event ] : Getting object " << name << " with automatic logic pass " << std::endl;
            return (ObjectType) getReal(name, "", true);
        }

        template<typename ObjectType> const ObjectType get(const std::string& name, const std::string& passName) {
            std::cout << "[ Event ] : Getting object " << name << " with pass " << passName << std::endl;
            return (ObjectType) getReal(name, passName, true);
        }

        virtual void add(const std::string& collectionName, TClonesArray* clones) = 0;
 
        virtual void add(const std::string& collectionName, TObject* obj) = 0;

        const TClonesArray* getCollection(const std::string& collectionName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with automatic logic" << std::endl;
            return (TClonesArray*) getReal(collectionName, "", true);
        }

        const TClonesArray* getCollection(const std::string& collectionName, std::string passName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with pass " << passName << std::endl;
            return (TClonesArray*) getReal(collectionName, passName, true);
        }

    protected:
  virtual const TObject* getReal(const std::string& itemName, const std::string& passName, bool mustExist) = 0;
  
};

}

#endif
