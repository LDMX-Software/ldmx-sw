/**
 * @file Event.h
 * @brief Class implementing an event buffer system for storing event data
 * @author Jeremy Mans, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef FRAMEWORK_EVENT_H_
#define FRAMEWORK_EVENT_H_

// ROOT
#include "TObject.h"
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

namespace ldmx {

    /**
     * @class Event
     * @brief Implements an event buffer system for storing event data
     *
     * @note
     * Event data is stored in ROOT trees and branches, which can be added
     * on the fly.  
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
            EventHeader &getEventHeader() {
                return eventHeader_;
            }

            /**
             * Check the existence of one-and-only-one object with the
             * given name (excluding the pass) in the event.
             * @param name Name (label, not class name) given to the object when it was put into the event.
             * @return True if the object or collection exists in the event.
             */
            bool exists(const std::string& name) const {
                return exists( name , "" );
            }

            /**
             * Check for the existence of an object or collection with the
             * given name and pass name in the event.
             * @param name Name (label, not class name) given to the object when it was put into the event.
             * @param passName The process pass label which was in use when this object was put into the event, such as "sim" or "rerecov2".
             * @return True if the object or collection *uniquely* exists in the event.
             */
            bool exists(const std::string& name, const std::string& passName) const {
                return ( searchProducts( name , passName , "" ).size() == 1 );
            }

            /**
             * Adds an object to the event bus
             * @param collectionName
             * @param obj in ROOT dictionary to add
             *
             * @note both the input type and the vector have to be included in the event root dictionary
             */
            template <typename T> void add( const std::string& collectionName, T &obj ) {
                if (collectionName.find('_') != std::string::npos) {
                    EXCEPTION_RAISE(
                            "IllegalName", 
                            "The product name '" + collectionName + "' is illegal as it contains an underscore.");
                }
        
                std::string branchName = makeBranchName(collectionName);
        
                if (branchesFilled_.find(branchName) != branchesFilled_.end()) {
                    EXCEPTION_RAISE(
                            "ProductExists", 
                            "A product named '" + collectionName + 
                            "' already exists in the event (has been loaded by a previous producer in this process.");
                }
                branchesFilled_.insert(branchName);
                auto itCollection = collections_.find(branchName);
                if (itCollection == collections_.end()) { 
                    // create a new branch for this collection
                    collections_[branchName] = EventBusPassenger( obj );
                    T *passengerAddress = &boost::get<T>(collections_[branchName]);
                    if (outputTree_ != 0) {
                        TBranch *outBranch = outputTree_->GetBranch( branchName.c_str() );
                        if ( outBranch ) {
                            //branch already exists, just reset branch address
                            outBranch->SetAddress( passengerAddress );
                        } else {
                            //branch doesnt exist, make new one
                            outBranch = outputTree_->Branch( branchName.c_str(), passengerAddress , 100000, 3);
                        }
                        newBranches_.push_back(outBranch);
                    }
        	        products_.push_back(ProductTag(collectionName,passName_,collections_[branchName].type().name()));
                    branchNames_.push_back(branchName);
                    knownLookups_.clear(); // have to invalidate this cache
                }
                
                //copy input contents into bus passenger
                EventBusPassenger toAdd( obj );
                if ( toAdd.which() == collections_[branchName].which() ) {
                    collections_[branchName] = toAdd;
                } else {
                    EXCEPTION_RAISE(
                            "TypeMismatch",
                            "Attempting to add an object whose type '" + std::string(toAdd.type().name()) + "' doesn't match the type stored in the collection '" +
                            std::string(collections_[branchName].type().name()) + "'"
                            );
                }

                return;
            }

            /**
	         * Get a list of products which match the given POSIX-Extended, case-insenstive regular-expressions.
	         * An empty argument is interpreted as ".*", which matches everything.
	         * @param namematch Regular expression to compare with the product name
	         * @param passmatch Regular expression to compare with the pass name
	         * @param typematch Regular expression to compare with the type name
	        */
            std::vector<ProductTag> searchProducts(
                    const std::string& namematch, const std::string& passmatch, const std::string& typematch) const;
      
            /**
             * Get a general object from the event bus
             */
            template <typename T>
            const T getObject(const std::string &collectionName, const std::string &passName) const {
                return getImpl<T>( collectionName , passName , true );
            }

            /**
             * Get a general object from the event bus when you don't care about the pass
             */
            template <typename T>
            const T getObject(const std::string &collectionName) const {
                return getObject<T>( collectionName , "" );
            }

            /**
             * Get a collection (std::vector) of objects from the event bus
             */
            template <typename T>
            const std::vector<T> getCollection(const std::string &collectionName, const std::string &passName ) const {
                return getObject< std::vector<T> >( collectionName , passName );
            }

            /**
             * Get a collection (std::vector) of objects from the event bus when you don't care about the pass
             */
            template <typename T>
            const std::vector<T> getCollection(const std::string &collectionName ) const {
                return getObject< std::vector<T> >( collectionName , "" );
            }

        protected:

            /**
             * Get an event passenger from the event bus (actual implementation)
             * @param collectionName name of collection you want
             * @param passName name of pass you want
             * @param mustExist flag to say whether or not you require this collection to exist in the tree
             */
            template <typename T> 
            T getImpl(const std::string& collectionName, const std::string& passName, bool mustExist) const {

                T retValIfNotFound;

                std::string branchName;
                if (collectionName== EventConstants::EVENT_HEADER) branchName=collectionName;
                else branchName = makeBranchName(collectionName, passName);
        
                if (passName.empty() && collectionName!= EventConstants::EVENT_HEADER) {
                    auto ptr=knownLookups_.find(collectionName);
                    if (ptr!=knownLookups_.end()) branchName=ptr->second;
                    else {
                        std::vector<std::vector<std::string>::const_iterator> matches;
                        branchName=collectionName+"_";
                        for (std::vector<std::string>::const_iterator ptr=branchNames_.begin(); ptr!=branchNames_.end(); ptr++) {
                            if (!ptr->compare(0,branchName.size(),branchName)) matches.push_back(ptr);
                        }
                        if (matches.empty()) {
                            if (!mustExist)
                                return retValIfNotFound;
                            EXCEPTION_RAISE(
                                    "ProductNotFound",
                                    "No product found for name '"+collectionName+"'");
                        } else if (matches.size()>1) {
                            std::string names;
                            for (auto strs : matches) {
                                if (!names.empty()) names+=", ";
                                names+=*strs;
                            }
                            if (!mustExist)
                                return retValIfNotFound;
                            EXCEPTION_RAISE(
                                    "ProductAmbiguous",
                                    "Multiple products found for name '"+collectionName+"' without specified pass name ("+names+")");
                        } else {
                            branchName=*matches.front();
                            knownLookups_[collectionName]=branchName;
                        }
                    }
                }
        
                //get iterators to branch and collection
                auto itb = branches_.find(branchName);
                auto ito = collections_.find(branchName);
        
                if (ito != collections_.end()) {
                   if (itb != branches_.end())
                      itb->second->GetEntry(ientry_);
                   return boost::get<T>(ito->second);
                } else if (inputTree_ == 0) {
                    EXCEPTION_RAISE(
                            "ProductNotFound", 
                            "No product found for name '" + collectionName + "' and pass '" + passName_ + "'");
                }
        
                // find the active branch and update if necessary
                if (itb != branches_.end()) {
        
                    // update buffers if needed
                    if (itb->second->GetReadEntry() != ientry_) {
        
                        TBranchElement* tbe = dynamic_cast<TBranchElement*>(itb->second);
                        if (!tbe)
                            itb->second->SetAddress( &boost::get<T>(ito->second) );
        
                        int nr = itb->second->GetEntry(ientry_, 1);
                    }
        
                    // check the objects map
                    if (ito != collections_.end())
                        return boost::get<T>(ito->second);
        
                    // this case is hard to achieve
                    return retValIfNotFound;
                } else {
        
                    // ok, maybe we've not loaded this yet, look for a branch
                    TBranch* branch = inputTree_->GetBranch(branchName.c_str());
                    if (branch == 0) {
                        EXCEPTION_RAISE(
                                "ProductNotFound", 
                                "No product found for name '" + collectionName + "' and pass '" + passName_ + "'");
                    }
                    // ooh, new branch!
                    branch->SetAutoDelete(false);
                    branch->SetStatus(1);
                    branch->GetEntry((ientry_<0)?(0):(ientry_));
                    //use default constructor to create new collection owned by this class
                    collections_[branchName] = EventBusPassenger( T() );
                    //get address of object that will be the event passenger
                    T *passengerAddress = &boost::get<T>( collections_[branchName] );
                    //connect input branch to this passenger
                    branch->SetAddress( passengerAddress );
        
                    branches_.insert(std::pair<std::string, TBranch*>(branchName, branch));
        
                    return *passengerAddress;
                }
            }

        public:

            /** ********* Functionality for storage  ********** **/

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
            EventHeader eventHeader_;

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
             * Map of names to collections.
             */
            mutable std::map<std::string, EventBusPassenger > collections_; 

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

#endif /* FRAMEWORK_EVENT_H_ */
