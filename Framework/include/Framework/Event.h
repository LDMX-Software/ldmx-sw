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

// LDMX
#include "Framework/Bus.h"
#include "Framework/EventHeader.h"
#include "Framework/Exception/Exception.h"
#include "Framework/ProductTag.h"

// STL
#include <regex.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace framework {

/**
 * @class Event
 * @brief Implements an event buffer system for storing event data
 *
 * Event data is stored in ROOT trees and branches for persistency.
 * For the buffering, we use a multi-layered inheritance tree that
 * is wrapped inside of the Bus class.
 * @see framework::Bus for this buffering tool
 */
class Event {
 public:
  /**
   * Class constructor.
   * @param passName The default pass name for adding event data.
   */
  Event(const std::string &passName);

  /**
   * Class destructor.
   */
  ~Event();

  /**
   * Get the event header.
   * @return A reference to the event header.
   */
  ldmx::EventHeader &getEventHeader() { return eventHeader_; }

  /**
   * Get the event header as a pointer
   * @return A const pointer to the event header.
   */
  const ldmx::EventHeader *getEventHeaderPtr() { return &eventHeader_; }

  /**
   * Get the event number.
   * @return the event index/number
   */
  int getEventNumber() const { return eventHeader_.getEventNumber(); }

  /**
   * Get the event weight
   * @return weight from the EventHeader
   */
  double getEventWeight() const { return eventHeader_.getWeight(); }

  /**
   * Print event bus
   *
   * Prints the list of products using the current stored product tags.
   */
  void Print() const;

  /**
   * Get a list of products which match the given POSIX-Extended,
   * case-insenstive regular-expressions.
   *
   * An empty argument is interpreted as ".*", which matches everything.
   *
   * By default (with full_string_match = false), the pattern can match
   * the full string or any substring within it. One can require the pattern
   * to match the full string by changing that parameter.
   *
   * @param namematch Regular expression to compare with the product name
   * @param passmatch Regular expression to compare with the pass name
   * @param typematch Regular expression to compare with the type name
   * @param full_string_match require all non-empty regular expressions to match
   * the full string and not a sub-string
   */
  std::vector<ProductTag> searchProducts(const std::string &namematch,
                                         const std::string &passmatch,
                                         const std::string &typematch,
                                         bool full_string_match = false) const;

  /**
   * Check for the existence of an object or collection with the
   * given name and pass name in the event.
   *
   * This function just uses the searchProducts function **while requiring
   * the input name and pass to match the full string**.
   *
   * @see searchProducts for a more flexible method for existence checking.
   * If you have any situation more complicated than simply checking if a
   * collection with exactly the input name, it is recommended to use
   * the searchProducts function directly. Within a processor, one can access
   * and print the products from a search relatively easily.
   * ```cpp
   * auto matches{event.searchProducts("MyCollection","","")};
   * std::cout << matches.size() << " event objects match pattern" << std::endl;
   * for (const auto& match : matches) { std::cout << match << std::endl; }
   * ```
   * searchProducts returns a list of ProductTag objects that store the name,
   * pass, and type of the objects matching the search.
   *
   * @param name Name (label, not class name) given to the object when it was
   * put into the event.
   * @param passName The process pass label which was in use when this object
   * was put into the event, such as "sim" or "rerecov2".
   * @param unique true if requiring one and only one matching object,
   * false if allowing for one or more matching objects
   * @return True if the object or collection exists in the event.
   */
  bool exists(const std::string &name, const std::string &passName = "",
              bool unique = true) const;

  /**
   * Add a drop rule to the list of regex expressions to drop.
   *
   * If a collection name matches one of the stored regex expressions, it will
   * be stored as a passenger but not added to output tree.
   *
   * @param exp regex to match
   */
  void addDrop(const std::string &exp);

  /**
   * Adds an object to the event bus
   *
   * @throws Exception if there is an underscore in the collection name.
   * @throws Exception if there already has been a branch filled with
   * the constructed name.
   * @throws Exception if the type we are putting in mis-matches the type in the
   * bus under the input name.
   *
   * @see makeBranchName
   * The branch name is constructed from the collection name
   * using the current pass name.
   *
   * @see Bus::board
   * @see Bus::attach
   * If the branch is not on the bus yet, we board the bus
   * and check if we need to attach the object to a tree.
   *
   * @see Bus::update
   * We always update the contents of the bus object
   * with the input object.
   *
   * @param collectionName
   * @param obj in ROOT dictionary to add
   */
  template <typename T>
  void add(const std::string &collectionName, T &obj) {
    if (collectionName.find('_') != std::string::npos) {
      EXCEPTION_RAISE("IllegalName",
                      "The product name '" + collectionName +
                          "' is illegal as it contains an underscore.");
    }

    // determine the branch name
    std::string branchName;
    if (collectionName == ldmx::EventHeader::BRANCH)
      branchName = collectionName;
    else
      branchName = makeBranchName(collectionName);

    if (branchesFilled_.find(branchName) != branchesFilled_.end()) {
      EXCEPTION_RAISE("ProductExists",
                      "A product named '" + collectionName +
                          "' already exists in the event (has been loaded by a "
                          "previous producer in this process).");
    }
    branchesFilled_.insert(branchName);
    // MEMORY add is leaking memory when given a vector (possible upon
    // destruction of Event?) MEMORY add is 'conditional jump or move depends on
    // uninitialised values' for all types of objects
    //  TTree::BranchImpRef or TTree::BronchExec
    if (not bus_.isOnBoard(branchName)) {
      // create a new branch for this collection

      // have type T board bus under name 'branchName'
      bus_.board<T>(branchName);

      // type name (want to use branch element if possible)
      std::string tname = typeid(obj).name();

      if (outputTree_ and not shouldDrop(branchName)) {
        // we are writing this branch to an output file, so let's
        //  attach this passenger to the output tree
        TBranch *outBranch = bus_.attach(outputTree_, branchName, true);
        // get type name from branch if possible,
        //  otherwise use compiler level type name (above)
        std::string class_name{outBranch->GetClassName()};
        if (not class_name.empty()) tname = class_name;
      }  // output tree exists or not

      // check for cache entry to remove
      auto it_known{knownLookups_.find(collectionName)};
      if (it_known != knownLookups_.end()) knownLookups_.erase(it_known);

      // add us to list of products
      products_.emplace_back(collectionName, passName_, tname);
    }

    // copy input contents into bus passenger
    try {
      bus_.update(branchName, obj);
    } catch (const std::bad_cast &) {
      EXCEPTION_RAISE("TypeMismatch",
                      "Attempting to add an object whose type '" +
                          std::string(typeid(obj).name()) +
                          "' doesn't match the type stored in the collection.");
    }

    return;
  }

  /**
   * Get an general object from the event bus
   *
   * First we determine the branch name. If the collection is the EventHeader
   * or if the passName is given, this is easy. But if the pass name is empty,
   * we try to find a matching collection under the list of branch names.
   * We will throw an exception if we don't find a unique branch corresponding
   * to the collection. We also employ a rudimentary caching system for mapping
   * collection names to branches (if no pass name is given) so this looping
   * only needs to happen once.
   * @see EventHeader::BRANCH for the name of the event header branch
   * @see makeBranchName for how we make branch names from object/pass names
   * @throws Exception if unable to uniquely determine the branch name
   * from the collection name alone.
   *
   * If the object we are asking for is already on the bus,
   * we simply make sure the branch corresponding to it is on the
   * correct entry.
   *
   * If the object we are asking for is *not* on the bus and there
   * is an input tree, we look for a branch on the input tree corresponding
   * to the branch name and then we load the current entry of that branch.
   * @throws Exception if can't find a corresponding branch name on
   * the input tree or in the bus
   * @throws Exception if we are trying to load a new branch and the input
   * tree has negative read entry (i.e. it is uninitialized)
   * @see Bus::board
   * @see Bus::attach
   *
   * Finally, we return a const reference to the object on the bus.
   * @throws Exception if mismatching type
   * @see Bus::get
   *
   * @tparam T type of object we should be getting
   * @param collectionName name of collection you want
   * @param passName name of pass you want
   * @return const reference to requested object
   */
  template <typename T>
  const T &getObject(const std::string &collectionName,
                     const std::string &passName = "") const {
    // get branch name
    std::string branchName;
    if (collectionName == ldmx::EventHeader::BRANCH) {
      branchName = collectionName;
    } else if (passName.empty()) {
      // if no passName, then find branchName by looking over known products
      if (knownLookups_.find(collectionName) == knownLookups_.end()) {
        // this collectionName hasn't been found before
        //   this collection name is the whole name and not a partial name
        //   so we search products with a full-string match required
        auto matches = searchProducts(collectionName, "", "", true);
        if (matches.empty()) {
          // no matches found
          EXCEPTION_RAISE("ProductNotFound",
                          "No product found for name '" + collectionName + "'");
        } else if (matches.size() > 1) {
          // more than one branch found
          std::stringstream names;
          for (auto strs : matches) {
            names << "\n" << strs;
          }
          EXCEPTION_RAISE("ProductAmbiguous",
                          "Multiple products found for name '" +
                              collectionName +
                              "' without specified pass name :" + names.str());
        } else {
          // exactly one branch found -> cache for later
          knownLookups_[collectionName] =
              makeBranchName(collectionName, matches.at(0).passname());
        }  // different options for number of possible branch matches
      }    // collection not in known lookups
      branchName = knownLookups_.at(collectionName);
    } else {
      branchName = makeBranchName(collectionName, passName);
    }

    // now we have determined the unique branch name to look for
    //  so we can start looking on the bus and the input tree
    //  (if it exists) for it
    bool already_on_board{bus_.isOnBoard(branchName)};
    if (not already_on_board and inputTree_) {
      // branch is not on the bus but there is an input tree
      //  -> let's look for a new branch to load

      // default construct a new passenger
      bus_.board<T>(branchName);

      // attempt to attach the new passenger to the input tree
      TBranch *branch = bus_.attach(inputTree_, branchName, false);
      if (branch == 0) {
        // inputTree doesn't have that branch
        EXCEPTION_RAISE("ProductNotFound", "No product found for branch '" +
                                               branchName + "' on input tree.");
      }
      // ooh, new branch!
      branch->SetStatus(1);  // overrides any 'ignore' rules
      /**
       * Load in the current entry
       *    This is necessary because getObject is called _after_
       *    EventFile::nextEvent loads the current entry of the inputTree.
       *    Many branches are turned "off" (status == 0), so they aren't
       *    loaded when the entire TTree is updated to a specific entry.
       *
       *    We shouldn't end up here before inputTree's read entry is unset,
       *    but we check anyways because ROOT will just seg-fault like a chump.
       */
      long long int ientry{inputTree_->GetReadEntry()};
      if (ientry < 0) {
        // reached getObject without initializing inputTree's read entry
        EXCEPTION_RAISE("InTreeInit",
                        "The input tree was un-initialized with read entry " +
                            std::to_string(ientry) +
                            " when attempting to get '" + branchName + "'.");
      }
      branch->GetEntry(ientry);
    } else if (not already_on_board) {
      // not found in loaded branches and there is no inputTree,
      // so no hope of finding an unloaded object
      EXCEPTION_RAISE("ProductNotFound", "No product found for name '" +
                                             collectionName + "' and pass '" +
                                             passName_ + "'");
    }

    // we've made sure the passenger is on the bus
    //  and the branch we are reading from (if we are reading)
    //  has been updated
    // let's return the object that the passenger is carrying
    try {
      const T &obj = bus_.get<T>(branchName);
      return obj;
    } catch (const std::bad_cast &) {
      EXCEPTION_RAISE("BadType", "Trying to get product from '" + branchName +
                                     "' but asking for wrong type.");
    }
  }  // getObject

  /**
   * Get a collection (std::vector) of objects from the event bus
   *
   * @see getObject for actual implementation
   *
   * @tparam[in,out] ContentType type of object stored in the vector
   * @param[in] collectionName name of collection that we want
   * @param[in] passName name of specific pass we want, optional
   * @returns const reference to collection of objects on the bus
   */
  template <typename ContentType>
  const std::vector<ContentType> &getCollection(
      const std::string &collectionName,
      const std::string &passName = "") const {
    return getObject<std::vector<ContentType> >(collectionName, passName);
  }

  /**
   * Get a map (std::map) of objects from the event bus
   *
   * @see getObject for actual implementation
   *
   * @tparam[in,out] KeyType type of object used as the key in the map
   * @tparam[in,out] ValType type of object used as the value in the map
   * @param[in] collectionName name of collection that we want
   * @param[in] passName name of specific pass we want, optional
   * @returns const reference to collection of objects on the bus
   */
  template <typename KeyType, typename ValType>
  const std::map<KeyType, ValType> &getMap(
      const std::string &collectionName,
      const std::string &passName = "") const {
    return getObject<std::map<KeyType, ValType> >(collectionName, passName);
  }

  /**
   * Set the input data tree.
   * @param tree The input data tree.
   */
  void setInputTree(TTree *tree);

  /**
   * Set the output data tree.
   * @param tree The output data tree.
   */
  void setOutputTree(TTree *tree);

  /**
   * Create the output data tree.
   * @return The output data tree.
   */
  TTree *createTree();

  /**
   * Get a list of the data products in the event
   */
  const std::vector<ProductTag> &getProducts() const { return products_; }

  /**
   * Go to the next event by retrieving the event header
   *
   * We deep-copy the event header into our own member object.
   * This is so we can modify the event header during the processing
   * of this event and then re-add the event header at the end of the event.
   * This is a pretty lame way to handle the event header, but it works.
   *
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
  std::string getPassName() { return passName_; }

  /** @return The beam electron count. */
  int getElectronCount() const { return electronCount_; }

  /**
   * Set the beam electron count.
   *
   * @param electronCount The beam electron count.
   */
  void setElectronCount(const int &electronCount) {
    electronCount_ = electronCount;
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
   * Make a branch name from a collection and pass name.
   * @param collectionName The collection name.
   * @param passName The pass name.
   */
  std::string makeBranchName(const std::string &collectionName,
                             const std::string &passName) const {
    return collectionName + "_" + passName;
  }

  /**
   * Make a branch name from a collection and the default(current) pass name.
   * @param collectionName The collection name.
   */
  std::string makeBranchName(const std::string &collectionName) const {
    return makeBranchName(collectionName, passName_);
  }

 private:
  /**
   * The event header object.
   */
  ldmx::EventHeader eventHeader_;

  /**
   * The default pass name.
   */
  std::string passName_;

  /**
   * The output tree for writing a new file.
   */
  TTree *outputTree_{nullptr};

  /**
   * The input tree for reading existing data.
   */
  TTree *inputTree_{nullptr};

  /// The total number of electrons in the event
  int electronCount_{1};

  /**
   * The Bus
   *
   * We buffer the event bus objects by having passengers
   * on the bus carry them.
   *
   * @see framework::Bus for how this buffering works
   */
  mutable framework::Bus bus_;

  /**
   * Names of branches filled during this event.
   *
   * This is used to make sure the same passenger isn't
   * modified more than once in one event _and_ to make
   * sure the event header is updated if it wasn't updated
   * manually.
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
}  // namespace framework

#endif /* FRAMEWORK_EVENT_H_ */
