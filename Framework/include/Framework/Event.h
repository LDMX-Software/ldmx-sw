/**
 * @file Event.h
 * @brief Class implementing an event buffer system for storing event data
 * @author Jeremy Mans, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef FRAMEWORK_EVENTIMPL_H_
#define FRAMEWORK_EVENTIMPL_H_

// ROOT
#include "TObject.h"
#include "TClonesArray.h"
#include "TTree.h"
#include "TBranch.h"
#include "TBranchElement.h"
#include "TBranchClones.h"

// LDMX
#include "Event/EventDef.h"
#include "Exception/Exception.h"

// STL
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <regex.h>

// Boost
#include <boost/variant.hpp>

namespace ldmx {

    typedef boost::variant< 
        EventHeader * ,
        SimParticle *  , 
        std::vector< SimParticle > *
        > EventObjectPtr;

    struct clearCollection : public boost::static_visitor<void> {
        void operator()(EventHeader *eh) const { eh->Clear(); return; }

        void operator()(SimParticle *sp) const { sp->Clear(); return; }

        template <class T>
        void operator()(std::vector<T> *vec) const { vec->clear(); return; }
    };

    /**
     * @class Event
     * @brief Implements an event buffer system for storing event data
     *
     * @note
     * Event data is stored in ROOT trees and branches, which can be added
     * on the fly.  The same TClonesArray and TObject pointers should be
     * used to add objects and collections from user code, as the class will
     * add a data structure for new ones automatically.
     */
    class Event {

        public:

            /**
             * Class constructor.
             * @param passName The default pass name for adding event data.
             */
            Event(const std::string& passName);

            /**
             * Class destructor.
             */
            ~Event();

            /**
             * Get the event header.
             * @return A constant copy of the event header.
             */
            const EventHeader* getEventHeader() const {
                return eventHeader_;
            }

            /**
             * Check the existence of one-and-only-one object with the
             * given name (excluding the pass) in the event.
             * @param name Name (label, not class name) given to the object when it was put into the event.
             * @return True if the object or collection exists in the event.
             */
            bool exists(const std::string& name) const {
                return getReal(name, "", false) != 0;
            }

            /**
             * Check for the existence of an object or collection with the
             * given name and pass name in the event.
             * @param name Name (label, not class name) given to the object when it was put into the event.
             * @param passName The process pass label which was in use when this object was put into the event, such as "sim" or "rerecov2".
             * @return True if the object or collection exists in the event.
             */
            bool exists(const std::string& name, const std::string& passName) const {
                return getReal(name, passName, false) != 0;
            }

            /**
             * Adds a clones array to the event/tree.
             * @param collectionName
             * @param tca The clones array to add.
             */
            void add(const std::string& collectionName, TClonesArray* tca);

            /**
             * Adds a vector of input type
             * @param collectionName
             * @param obj in ROOT dictionary to add
             *
             * @note both the input type and the vector have to be included in the event root dictionary
             */
            template <typename T> void addCollection( const std::string& collectionName, T &obj ) {
        
                if (collectionName.find('_') != std::string::npos) {
                //    EXCEPTION_RAISE("IllegalName", "The product name '" + collectionName + "' is illegal as it contains an underscore.");
                }
        
                std::string branchName = makeBranchName(collectionName);
        
                if (branchesFilled_.find(branchName) != branchesFilled_.end()) {
                 //   EXCEPTION_RAISE("ProductExists", "A product named '" + collectionName + "' already exists in the event (has been loaded by a previous producer in this process.");
                }
                branchesFilled_.insert(branchName);
        
                auto itCollection = collections_.find(branchName);
                if (itCollection == collections_.end()) { 
                    // create a new branch for this collection
                    collections_[branchName] = (EventObjectPtr)(&obj);
                    std::cout << collections_[branchName].type().name();
                    if (outputTree_ != 0) {
                        TBranch *outBranch = outputTree_->GetBranch( branchName.c_str() );
                        if ( outBranch ) {
                            //branch already exists, just reset branch address
                            outBranch->SetAddress( boost::get< T *>(collections_[branchName]) );
                        } else {
                            //branch doesnt exist, make new one
                            outBranch = outputTree_->Branch(
                                    branchName.c_str(), boost::get<T *>(collections_[branchName]) , 100000, 3);
                        }
                        newBranches_.push_back(outBranch);
                    }
        	        products_.push_back(ProductTag(collectionName,passName_,collections_[branchName].type().name()));
                    branchNames_.push_back(branchName);
                    knownLookups_.clear(); // have to invalidate this cache
                }
            }

            /**
             * Adds a general object to the event/tree.
             * @param name The name of the object.
             * @param obj The object to add.
             *
             * @note
             * All objects must implement/replace TObject::Clone() to
             * simply call "new" and create an empty new object and
             * implement TObject::Copy() to either copy the contents of
             * the object or swap them to the calling function, which
             * is more efficient.
             */
            void add(const std::string& name, TObject* obj);

            /**
             * Add the given object to the named TClonesArray collection
             * Objects can only be added to a TClonesArray during the current pass -- TClonesArrays loaded
             * from the input data file are not allowed to be changed.
             * @note Object types must implement TObject::Copy()
             * @param name Name of the collection
             * @param obj Object to be appended to the collection
             */
            void addToCollection(const std::string& name, const TObject& obj);

            /**
	     * Get a list of products which match the given POSIX-Extended, case-insenstive regular-expressions.
	     * An empty argument is interpreted as ".*", which matches everything.
	     * @param namematch Regular expression to compare with the product name
	     * @param passmatch Regular expression to compare with the pass name
	     * @param typematch Regular expression to compare with the type name
	     */
            std::vector<ProductTag> searchProducts(const std::string& namematch, const std::string& passmatch, const std::string& typematch) const;
      
            /**
             * Get a named object with a specific type without specifying
             * the pass name.  If there is one-and-only-one object with the
             * given name (excluding the pass) in the event, it will be
             * returned, otherwise an exception will be thrown.
             * @param name Name (label, not classname) given to the object when it was put into the event.
             * @return A named object from the event.
             */
            template<typename ObjectType> const ObjectType get(const std::string& name) const {
                return (ObjectType) getReal(name, "", true);
            }

            /**
             * Get a named object with a specific type, specifying
             * the pass name.  If there is no object which matches, an exception
             * will be thrown.
             * @param name Name (label, not classname) given to the object when it was put into the event.
             * @param passName The process pass label which was in use when this object was put into the event, such as "sim" or "rerecov2".
             * @return A named object from the event.
             */
            template<typename ObjectType> const ObjectType get(const std::string& name, const std::string& passName) const {
                return (ObjectType) getReal(name, passName, true);
            }

            /**
             * Get a collection (TClonesArray) from the event without specifying
             * the pass name.  If there is one-and-only-one object with the
             * given name (excluding the pass) in the event, it will be
             * returned, otherwise an exception will be thrown.
             * @param name Name (label, not class name) given to the object when it was put into the event.
             * @return The named TClonesArray from the event.
             */
            const TClonesArray* getCollection(const std::string& collectionName) const {
                return (TClonesArray*) getReal(collectionName, "", true);
            }

            /**
             * Get an object collection (TClonesArray) from the event, specifying
             * the pass name.  If there is no object which matches, an exception
             * will be thrown.
             * @param collectionName Name given to the collection when it was put into the event.
             * @param passName The process pass label which was in use when this object was put into the event, such as "sim" or "rerecov2".
             * @return A named TClonesArray from the event.
             */
            const TClonesArray* getCollection(const std::string& collectionName, std::string passName) const {
                return (TClonesArray*) getReal(collectionName, passName, true);
            }
        protected:

            /**
             * Get an object from the event using a custom pass name.
             * @param collectionName The collection name.
             * @param passName The pass name.
             */
            const TObject* getReal(const std::string& collectionName, const std::string& passName, bool mustExist) const;

        public:

            /** ********* Functionality for storage  ********** **/

            /**
             * Get a mutable copy of the EventHeader object.
             * @return A mutable copy of the EventHeader object.
             */
            EventHeader& getEventHeaderMutable() const {
                return *eventHeader_;
            }

            /**
             * Set the input data tree.
             * @param tree The input data tree.
             */
            void setInputTree(TTree* tree);

            /**
             * Set the output data tree.
             * @param tree The output data tree.
             */
            void setOutputTree(TTree* tree);

            /**
             * Create the output data tree.
             * @return The output data tree.
             */
            TTree* createTree();

            /**
             * Make a branch name from a collection and pass name.
             * @param collectionName The collection name.
             * @param passName The pass name.
             */
            std::string makeBranchName(const std::string& collectionName, const std::string& passName) const {
                return collectionName + "_" + passName;
            }

            /**
             * Make a branch name from a collection and the default(current) pass name.
             * @param collectionName The collection name.
             */
            std::string makeBranchName(const std::string& collectionName) const {
                return makeBranchName(collectionName, passName_);
            }

   	        /**
             * Get a list of the data products in the event
	         */
	        const std::vector<ProductTag>& getProducts() const { return products_; }
      
            /**
             * Go to the next event by incrementing the entry index.
             * @return Hard-coded to return true.
             */
            bool nextEvent();

            /**
             * Action to be executed before the tree is filled.
             */
            void beforeFill();

            /**
             * Clear this object's data.
             */
            void Clear();

            /**
             * Perform end of event action (clears the owned objects).
             */
            void onEndOfEvent();

            /**
             * Perform end of file action (doesn't do anything right now).
             */
            void onEndOfFile();

            /**
             * Get the current/default pass name.
             * @return The current/default pass name.
             */
            std::string getPassName() {
                return passName_;
            }

        private:

            /**
             * The event header object (as pointer).
             */
            EventHeader* eventHeader_{nullptr};

            /**
             * Number of entries in the tree.
             */
            Long64_t entries_{-1};

            /**
             * Current entry in the tree.
             */
            Long64_t ientry_{-1};

            /**
             * The default pass name.
             */
            std::string passName_;

            /**
             * The output tree for writing a new file.
             */
            TTree* outputTree_{nullptr};

            /**
             * The input tree for reading existing data.
             */
            TTree* inputTree_{nullptr};

            /**
             * Map of names to branches.
             */
            mutable std::map<std::string, TBranch*> branches_;

            /**
             * Map of names to objects.
             */
            mutable std::map<std::string, TObject*> objects_;

            /**
             * Map of names to collections.
             */
            mutable std::map<std::string, EventObjectPtr > collections_; 

            /**
             * Map of owned objects that should eventually be cleared at end of event
             * and deleted when this object is destroyed.
             */
            std::map<std::string, TObject*> objectsOwned_;

            /**
             * List of new branches added.
             */
            std::vector<TBranch*> newBranches_;

            /**
             * Names of all branches.
             */
            std::vector<std::string> branchNames_;

            /**
             * Names of branches filled during this event.
             */
            std::set<std::string> branchesFilled_;

            /**
             * Efficiency cache for empty pass name lookups.
             */
            mutable std::map<std::string, std::string> knownLookups_;
	
            /**
             * List of all the event products
             */
            std::vector<ProductTag> products_;
    }; 
}

#endif
