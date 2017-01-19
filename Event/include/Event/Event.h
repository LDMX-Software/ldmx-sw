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

/**
 * @class Event
 * @brief Defines an interface for accessing event data
 *
 * @note
 * A backing EventImpl object provides the actual data collections
 * via ROOT data structures (trees and branches).  
 */
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
        
	/**
	 * Get the event header.
         * @return The event header.
         */
        virtual const EventHeader* getEventHeader() const = 0;
	
	/**
         * Check the existance of one-and-only-one object with the
         * given name (excluding the pass) in the event.
	 * @param name Name (label, not classname) given to the object when it was put into the event
	 * @return True if the object or collection exists in the event
         */
        bool exists(const std::string& name) {
            return getReal(name, "", false)!=0;
        }

	/**
         * Check the existance of an object or collection with the
         * given name and pass name in the event.
	 * @param name Name (label, not classname) given to the object when it was put into the event
	 * @param passName The process pass label which was in use when this object was put into the event, such as "sim" or "rerecov2"
	 * @return True if the object or collection exists in the event
         */
        bool exists(const std::string& name, const std::string& passName) {
            return getReal(name, passName, false)!=0;
        }

	/**
         * Get a named object with a specific type without specifying
         * the pass name.  If there is one-and-only-one object with the
         * given name (excluding the pass) in the event, it will be
         * returned, otherwise an exception will be thrown.
	 * @param name Name (label, not classname) given to the object when it was put into the event
	 * @return A named object from the event.
         */
        template<typename ObjectType> const ObjectType get(const std::string& name) {
            std::cout << "[ Event ] : Getting object " << name << " with automatic logic pass " << std::endl;
            return (ObjectType) getReal(name, "", true);
        }

	/**
         * Get a named object with a specific type, specifying
         * the pass name.  If there is no object which matches, an exception
         * will be thrown.
	 * @param name Name (label, not classname) given to the object when it was put into the event
	 * @param passName The process pass label which was in use when this object was put into the event, such as "sim" or "rerecov2"
	 * @return A named object from the event.
         */
        template<typename ObjectType> const ObjectType get(const std::string& name, const std::string& passName) {
            std::cout << "[ Event ] : Getting object " << name << " with pass " << passName << std::endl;
            return (ObjectType) getReal(name, passName, true);
        }

	/**
         * Get a collection (TClonesArray) from the event without specifying
         * the pass name.  If there is one-and-only-one object with the
         * given name (excluding the pass) in the event, it will be
         * returned, otherwise an exception will be thrown.
	 * @param name Name (label, not classname) given to the object when it was put into the event
	 * @return The named TClonesArray from the event.
         */
        const TClonesArray* getCollection(const std::string& collectionName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with automatic logic" << std::endl;
            return (TClonesArray*) getReal(collectionName, "", true);
        }

	/**
         * Get an object collection (TClonesArray) from the event, specifying
         * the pass name.  If there is no object which matches, an exception
         * will be thrown.
	 * @param collectionName Name given to the collection when it was put into the event
	 * @param passName The process pass label which was in use when this object was put into the event, such as "sim" or "rerecov2"
	 * @return A named TClonesArray from the event.
         */
        const TClonesArray* getCollection(const std::string& collectionName, std::string passName) {
            std::cout << "[ Event ] : Getting collection " << collectionName << " with pass " << passName << std::endl;
            return (TClonesArray*) getReal(collectionName, passName, true);
        }

	/**
         * Add a collection (TClonesArray) of objects to the event.
         * The current pass name will be used for the collection.
         * @param collectionName The name of the collection.
         * @param clones The TClonesArray containing the objects.
         */
        virtual void add(const std::string& collectionName, TClonesArray* clones) = 0;
 
	/**
         * Add an object to the event.
         * The current pass name will be used for the object.
         * @param collectionName The name of the collection.
         * @param clones The TClonesArray containing the objects.
         */
        virtual void add(const std::string& name, TObject* obj) = 0;
	
    protected:
	
	/**
         * Actual get implementation, provided by derived class.
         * @param itemName The name of the object or TClonesArray
	 * @param passName The process pass label which was in use when this object was put into the event
	 * @param mustExist Determines if an exception should be thrown if the object does not exist -- used by exists() methods
         * @param clones The TClonesArray containing the objects.
         */
	virtual const TObject* getReal(const std::string& itemName, const std::string& passName, bool mustExist) = 0;
	
    };   
}

#endif
