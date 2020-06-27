/**
 * @file Event.h
 * @brief Class implementing an event buffer system for storing event data
 * @author Jeremy Mans, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef FRAMEWORK_EVENT_H_
#define FRAMEWORK_EVENT_H_

// ROOT
#include "TTree.h"
#include "TBranchElement.h"

// LDMX
#include "Event/EventDef.h"
#include "Exception/Exception.h"

// STL
#include <algorithm> 
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <variant>
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
             * @return A reference to the event header.
             */
            EventHeader &getEventHeader() {
                return eventHeader_;
            }

            /**
             * Get the event number.
             * @return the event index/number
             */
            int getEventNumber() const {
                return eventHeader_.getEventNumber();
            }

            /**
             * Get the event weight
             * @return weight from the EventHeader
             */
            double getEventWeight() const {
                return eventHeader_.getWeight();
            }

            /**
             * Print event bus
             *
             * Only prints passengers that have been loaded into the member object.
             * This means what is printed depends on when this method is called.
             * For example, if this method is called after the Clear method, then
             * everything will be an empty object.
             *
             * @param verbosity integer flag to determine how verbose you want to be.
             */
            void Print(int verbosity) const;

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
             * Add a drop rule to the list of regex expressions to drop.
             *
             * If a collection name matches one of the stored regex expressions, it will be stored as a passenger 
             * but not added to output tree.
             *
             * @param exp regex to match
             */
            void addDrop( const std::string &exp );

            /**
             * Adds an object to the event bus
             * @param collectionName
             * @param obj in ROOT dictionary to add
             */
            template <typename T> 
            void add( const std::string& collectionName, T &obj ) {
                if (collectionName.find('_') != std::string::npos) {
                    EXCEPTION_RAISE(
                            "IllegalName", 
                            "The product name '" 
                            + collectionName 
                            + "' is illegal as it contains an underscore.");
                }
        
                std::string branchName;
                if (collectionName== EventConstants::EVENT_HEADER) branchName=collectionName;
                else branchName = makeBranchName(collectionName);
        
                if (branchesFilled_.find(branchName) != branchesFilled_.end()) {
                    EXCEPTION_RAISE(
                            "ProductExists", 
                            "A product named '" 
                            + collectionName 
                            + "' already exists in the event (has been loaded by a previous producer in this process)."
                            );
                }
                branchesFilled_.insert(branchName);
                //MEMORY add is leaking memory when given a vector (possible upon destruction of Event?)
                //MEMORY add is 'conditional jump or move depends on uninitialised values' for all types of objects
                //  TTree::BranchImpRef or TTree::BronchExec
                if (passengers_.find(branchName) == passengers_.end()) { 
                    // create a new branch for this collection
                    //TODO check if input type is listed as an EventBusPassenger?
                    passengers_[branchName] = EventBusPassenger( obj );
                    T *passengerAddress = std::get_if<T>(&passengers_[branchName]);
                    std::string tname = typeid(obj).name();//type name (want to use branch element if possible)
                    if (outputTree_ != 0 and not shouldDrop( branchName ) ) {
                        TBranchElement *outBranch = dynamic_cast<TBranchElement *>(outputTree_->GetBranch( branchName.c_str() ));
                        if ( outBranch ) {
                            //branch already exists, just reset branch object
                            outBranch->SetObject( passengerAddress );
                        } else {
                            //branch doesnt exist, make new one
                            outBranch = dynamic_cast<TBranchElement*>(outputTree_->Branch( branchName.c_str(), passengerAddress , 100000, 3));
                        }
                        newBranches_.push_back(outBranch);
                        //get type name from branch if possible, otherwise use compiler level type name (above)
                        tname = outBranch->GetClassName();
                    } //output tree exists or not
        	        products_.emplace_back(collectionName,passName_,tname);
                    branchNames_.push_back(branchName);
                    knownLookups_.clear(); // have to invalidate this cache
                }
                
                //copy input contents into bus passenger
                EventBusPassenger toAdd( obj );
                if ( toAdd.index() == passengers_[branchName].index() ) {
                    std::visit( sortPassenger() , toAdd ); //sort before copying over
                    passengers_[branchName] = toAdd;
                } else {
                    EXCEPTION_RAISE(
                            "TypeMismatch",
                            "Attempting to add an object whose type '" 
                            + std::string(typeid(obj).name()) 
                            + "' doesn't match the type stored in the collection."
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
            std::vector<ProductTag> searchProducts( const std::string& namematch, const std::string& passmatch, const std::string& typematch) const;
      
            /**
             * Get a general object from the event bus
             */
            template <typename T>
            const T getObject(const std::string &collectionName, const std::string &passName) const {
                return getImpl<T>( collectionName , passName );
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
                return getCollection<T>( collectionName , "" );
            }

            /**
             * Get a map (std::map) of objects from the event bus
             */
            template <typename Key, typename Val>
            const std::map<Key,Val> getMap(const std::string &collectionName, const std::string &passName ) const {
                return getObject< std::map<Key,Val> >( collectionName , passName );
            }

            /**
             * Get a map of objects from the event bus when you don't care about the pass
             */
            template <typename Key, typename Val>
            const std::map<Key,Val> getMap(const std::string &collectionName ) const {
                return getMap<Key,Val>( collectionName , "" );
            }

        protected:

            /**
             * Get an event passenger from the event bus (actual implementation)
             * @param collectionName name of collection you want
             * @param passName name of pass you want
             */
            template <typename T> 
            T getImpl(const std::string& collectionName, const std::string& passName) const {

                //get branch name
                std::string branchName;
                if (collectionName== EventConstants::EVENT_HEADER) branchName=collectionName;
                else branchName = makeBranchName(collectionName, passName);
        
                //if no passName, then find branchName by looking over known branches
                if (passName.empty() && collectionName!= EventConstants::EVENT_HEADER) {
                    auto itKL = knownLookups_.find(collectionName);
                    if (itKL!=knownLookups_.end()) branchName=itKL->second;
                    else {
                        //this collecitonName hasn't been found before
                        std::vector<std::vector<std::string>::const_iterator> matches;
                        branchName=collectionName+"_";
                        for (auto itBN=branchNames_.begin(); itBN!=branchNames_.end(); itBN++) {
                            if (!itBN->compare(0,branchName.size(),branchName)) matches.push_back(itBN);
                        }
                        if (matches.empty()) {
                            //no matches found
                            EXCEPTION_RAISE(
                                    "ProductNotFound",
                                    "No product found for name '"
                                    + collectionName
                                    + "'"
                                    );
                        } else if (matches.size()>1) {
                            //more than one branch found
                            std::string names;
                            for (auto strs : matches) {
                                if (!names.empty()) names+=", ";
                                names+=*strs;
                            }
                            EXCEPTION_RAISE(
                                    "ProductAmbiguous",
                                    "Multiple products found for name '"
                                    + collectionName
                                    + "' without specified pass name ("
                                    + names
                                    + ")"
                                    );
                        } else {
                            //exactly one branch found
                            branchName=*matches.front();
                            knownLookups_[collectionName]=branchName;
                        }
                    }
                }
       
                //get iterators to branch and collection
                auto itBranch = branches_.find(branchName);
                auto itPassenger = passengers_.find(branchName);
        
                if (itPassenger != passengers_.end()) {
                    if (itBranch != branches_.end()) {
                        //passenger and branch found
                        itBranch->second->GetEntry(ientry_);
                        //reading branches from input tree need to be manually updated
                        passengers_[branchName] = *((T *)(itBranch->second->GetObject()));
                    }
                    return std::get<T>(itPassenger->second);
                } else if (inputTree_ == 0) {
                    //not found in loaded branches and there is no inputTree,
                    // so no hope of finding an unloaded object
                    EXCEPTION_RAISE(
                            "ProductNotFound", 
                            "No product found for name '" 
                            + collectionName 
                            + "' and pass '" 
                            + passName_ 
                            + "'");
                }
        
                // find the active branch and update if necessary
                if (itBranch != branches_.end()) {
        
                    // update buffers if needed
                    if (itBranch->second->GetReadEntry() != ientry_) {
                        itBranch->second->GetEntry(ientry_, 1);
                    }
        
                    // check the objects map
                    if (itPassenger != passengers_.end())
                        return std::get<T>(itPassenger->second);
        
                    // this case is hard (impossible?) to achieve
                    EXCEPTION_RAISE(
                            "ProductNotFound", 
                            "A branch mis-match occurred. I'm not sure how I got here!" 
                            );
                } else {
        
                    // ok, maybe we've not loaded this yet, look for a branch
                    TBranchElement* branch = dynamic_cast<TBranchElement *>(inputTree_->GetBranch(branchName.c_str()));
                    if (branch == 0) {
                        //inputTree doesn't have that branch
                        EXCEPTION_RAISE(
                                "ProductNotFound", 
                                "No product found for name '" 
                                + collectionName 
                                + "' and pass '" 
                                + passName_ 
                                + "'"
                                );
                    } 
                    // ooh, new branch!
                    // load in the current entry
                    branch->SetStatus(1); //overrides any 'ignore' rules
                    branch->GetEntry((ientry_<0)?(0):(ientry_));
        
                    //insert into maps of loaded branches and passengers
                    passengers_[branchName] = *((T *)(branch->GetObject())); //this will fail if wrong type is passed
                    branches_[branchName]   = branch;
        
                    return std::get<T>( passengers_.at(branchName) );
                }
            } //getImpl

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
             * Clear this object's data (including passengers).
             */
            void Clear();

            /**
             * Perform end of event action (doesn't do anything right now).
             */
            void onEndOfEvent();

            /**
             * Perform end of file action.
             *
             * Clears buffer objects and resets output branch addresses.
             * This prepares the event bus for a new input file (with new addresses).
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
             * Check if collection should be dropped.
             *
             * @param collName name of collection
             * @return true if the collection should be dropped (i.e. NOT saved)
             */
            bool shouldDrop(const std::string &collName) const;
        
            /**
             * @class clearPassenger
             * Clearing of event objects.
             *
             * This is necessary, so if a producer skips an event, then
             * the last object added won't be filled into event tree another time.
             */
            class clearPassenger {
                public:
                    
                    /**
                     * All vector passengers can be cleared in the same way.
                     */
                    template <typename T>
                    void operator()(std::vector<T> &vec) const { vec.clear(); }
        
                    /**
                     * All map passengers can be cleared in the same way.
                     */
                    template <typename Key, typename Val>
                    void operator()(std::map<Key,Val> &m) const { m.clear(); }
        
                    /**
                     * Right now all other event objects have a Clear method defined.
                     *
                     * @note Notice the difference in capitalization.
                     * This is a artefact of inheriting from TObject.
                     */
                    template <typename T>
                    void operator()(T &obj) const { obj.Clear(); }
        
            };
        
            /**
             * @class sortPassenger
             * Sorting of passenger event objects.
             *
             * This method allows for the collections to be sorted by
             * the content's defined comparison operator <.
             *
             * @note If the operator < is not defined, std::sort implicitly
             * converts the object to a more basic object that has an included
             * less than operator. If this implicit conversion doesn't work,
             * then a complicated compile error is thrown.
             *
             * @note More specific sorting methods can be input here if you wish.
             * When templating, be aware that the order does matter. std uses
             * the first function that matches the input type. (From top down,
             * so input a specific sorting method at the top).
             */
            class sortPassenger {
                public:
                    /**
                     * Sort vectors using the std::sort method.
                     */
                    template <typename T>
                    void operator()(std::vector<T> &vec) const { std::sort(vec.begin(),vec.end()); }
        
                    /**
                     * Don't sort the other objects.
                     *
                     * The unused attribute requires the compiler to be gcc!
                     */
                    template <typename T>
                    void operator()(__attribute__((unused)) T &obj) const { /*Nothing on purpose*/ return; }
        
            };
        
        
            /**
             * @class printPassenger
             * Printing of event objects.
             *
             * This method requires all event objects to have a Print method defined.
             */
            class printPassenger {
                public:
                    
                    /**
                     * Constructor
                     *
                     * Sets verbosity
                     */
                    printPassenger(int verbosity) : verbosity_(verbosity) { }
        
                    /**
                     * Prints size and contents of all vectors depending on verbosity.
                     */
                    template <typename T>
                    void operator()(const std::vector<T> &vec) const { 
                        if ( verbosity_ > 1 ) {
                            std::cout << "Size: " << vec.size() << std::endl;
                        }
                        if ( verbosity_ > 2 ) {
                            std::cout << "Contents:" << std::endl;
                            for ( const T &obj : vec ) {
                                std::cout << "    ";
                                obj.Print();
                            }
                            std::cout << std::endl;
                        }
                    }
        
                    /**
                     * Prints size and contents of all maps depending on verbosity.
                     */
                    template <typename Key, typename Val>
                    void operator()(const std::map<Key,Val> &m) const { 
                        if ( verbosity_ > 1 ) {
                            std::cout << "Size: " << m.size() << std::endl;
                        }
                        if ( verbosity_ > 2 ) {
                            std::cout << "Contents:" << std::endl;
                            for ( const auto &keyVal : m ) {
                                std::cout << "    " << keyVal.first << " -> ";
                                keyVal.second.Print();
                            }
                            std::cout << std::endl;
                        }
                    }
        
                    /**
                     * Just prints the object if verbosity is nonzero.
                     */
                    template <typename T>
                    void operator()(const T &obj) const { 
                        if ( verbosity_ > 1 ) obj.Print(); 
                    }
        
                private:
                    /** Flag for how much to print */
                    int verbosity_;
        
            };

        private:

            /**
             * The event header object.
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
            mutable std::map<std::string, TBranchElement*> branches_;

            /**
             * Map of names to passengers.
             */
            mutable std::map<std::string, EventBusPassenger > passengers_; 

            /**
             * List of new branches added.
             */
            std::vector<TBranchElement*> newBranches_;

            /**
             * Names of all branches.
             */
            std::vector<std::string> branchNames_;

            /**
             * Names of branches filled during this event.
             */
            std::set<std::string> branchesFilled_;

            /**
             * Regex of collection names to *not* store in event.
             */
            std::vector<regex_t> regexDropCollections_;

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
