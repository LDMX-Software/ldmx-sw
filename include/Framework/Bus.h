#ifndef FRAMEWORK_BUS_H
#define FRAMEWORK_BUS_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ROOT
#include "TBranchElement.h"
#include "TTree.h"

namespace framework {

/**
 * A map of bus passengers
 *
 * This is the actual bus that the passengers ride on
 * during event processing. At its core, it is simply
 * a specialization of a map between branch names and
 * handles to the bus passengers (Seats) with some special
 * accessors for connecting TTrees and updating contents.
 *
 * @see Bus::Seat for how we keep a handle on the Passengers
 * @see Bus::Passenger for what actually carries the event objects
 */
class Bus {
 public:
  /**
   * Get the baggage carried by the passenger with the passed name.
   *
   * @see getRef for getting the passenger
   * @throws std::bad_cast if BaggageType does not match type of object
   * passenger is carrying
   *
   * @tparam[in] BaggageType type of object carried by passenger
   * @param[in] name Name of Passenger (corresponds to branch_name)
   * @return const reference to object carried by passenger
   */
  template <typename BaggageType>
  const BaggageType& get(const std::string& name) {
    return getRef<BaggageType>(name).get();
  }

  /**
   * Board a new passenger onto the bus
   *
   * @note Does not check if we are overwriting any other passenger!
   *
   * Creates a new passenger that carries an object of
   * type BaggageType and puts this passenger into the map of passengers.
   *
   * @note If you are seeing some funky "Clear not defined" compiling
   * error while attempting to use a newly created event bus object,
   * you should check that the class you have written matches the
   * requirements for any of the bus types in the documentation
   * for Bus::Passenger.
   *
   * @tparam[in] BaggageType type of object new passenger is carrying
   * @param[in] name Name of new passenger (corresponds to branch name)
   */
  template <typename BaggageType>
  void board(const std::string& name) {
    passengers_[name] = std::make_unique<Passenger<BaggageType>>();
    passengers_[name]->clear();  // make sure 'default' state is well defined
  }

  /**
   * Update the object a passenger is carrying
   *
   * @see getRef for getting the passenger
   * @see Passenger::update for how we update a passenger
   * @throws std::bad_cast if BaggageType does not match type of object
   * passenger is carrying
   *
   * @tparam[in] BaggageType type of object carried by passenger
   * @param[in] name name of passenger (corresponds to branch name)
   * @param[in] obj update object that should be carried by passenger
   */
  template <typename BaggageType>
  void update(const std::string& name, const BaggageType& obj) {
    getRef<BaggageType>(name).update(obj);
  }

  /**
   * Attach the input tree to the object a passenger is carrying
   *
   * @note Does not check if passenger exists in the map of passengers.
   * This means it would get pretty messy if you try to attach a TTree
   * to a passenger that is the wrong type or is not already on the bus.
   *
   * @note Does not check if branch on the tree is expecting the type
   * that is carried by the passenger. Don't know how this will affect
   * things, but it tends to produce a seg fault because serializing
   * objects between different types gets very messy.
   *
   * @see Passenger::attach for how we attach a passenger
   *
   * @param[in] tree pointer to TTree to attach to
   * @param[in] name name of passenger (and branch of tree)
   * @param[in] can_create true if we are allowed to create new branches on the
   * tree
   * @returns pointer to branch that we attached to (may be null)
   */
  TBranch* attach(TTree* tree, const std::string& name, bool can_create) {
    return passengers_[name]->attach(tree, name, can_create);
  }

  /**
   * Check if a passenger is on the bus
   *
   * @param[in] name name of passenger to check for
   * @return true if name is a key in the map
   */
  bool isOnBoard(const std::string& name) {
    return passengers_.find(name) != passengers_.end();
  }

  /**
   * Reset the objects carried by the passengers
   *
   * @see Passenger::clear for how we clear the individual passengers
   */
  void clear() {
    for (auto& [n, handle] : passengers_) handle->clear();
  }

  /**
   * Kicks all of the passengers off the bus
   * and therefore destroys any objects they
   * are carrying.
   *
   * @note This is where the Passengers are destructed
   * and therefore their baggage is deleted.
   * @see Bus::Passenger::~Passenger for comments
   * about why you need to be careful.
   */
  void everybodyOff() { passengers_.clear(); }

  /**
   * Write the bus to the input ostream.
   *
   * Includes new-line characters to separate out the different
   * objects carried on the bus.
   *
   * @param[in] s ostream to write to
   */
  void stream(std::ostream &s) const {
    for (auto& [n, handle] : passengers_)
      s << n << " : " << handle << std::endl;
  }

  /**
   * Allow for the bus to be streamed to an ostream
   *
   * @see stream for the implmentation
   *
   * @param[in] s ostream to write to
   * @param[in] b Bus to write out
   * @return modified ostream
   */
  friend std::ostream &operator<<(std::ostream &s, const framework::Bus &b) {
    b.stream(s);
    return s;
  }

 private:
  /**
   * The handle of a bus passenger
   *
   * This is the foundation of the bus. Each bus passenger (which
   * does know the type of object it is carrying) is given a seat
   * on the bus that doesn't know the type of object it is carrying.
   * This allows the bus to have one container for all the passengers
   * carrying different types of objects and tell each of these objects
   * to reset themselves at the end of the event.
   */
  class Seat {
   public:
    /**
     * Destructor
     *
     * Virtual so that the derived classes we will be pointing
     * to with this class will be destructed when this is destructed.
     */
    virtual ~Seat() {}

    /**
     * Attach this passenger to the input tree.
     *
     * We don't define the attachement here, but this
     * pure virtual function allows for us to make the attachment
     * without knowledge of the type of object the derived class
     * is storing.
     *
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create if true, we can create a new branch if we don't
     * find one
     * @returns pointer to branch that we attached to (maybe be null if no
     * branch found)
     */
    virtual TBranch* attach(TTree* tree, const std::string& branch_name,
                            bool can_create) = 0;

    /**
     * Clear this passenger
     *
     * Basically, we need to allow for this handle to reset
     * the contents of the passenger because we won't know the type
     * that is being stored when we wish to reset the event bus
     * to an "undefined" state.
     */
    virtual void clear() = 0;

    /**
     * Define how we should stream the object to the input stream.
     *
     * @param[in] s ostream to write to
     */
    virtual void stream(std::ostream &s) const = 0;

    /**
     * Stream this object to the output stream
     *
     * Notice that the input Seat is a uniqe_ptr.
     * This is because that is what is stored in the Bus.
     *
     * May contain newlines if large object.
     *
     * @see stream for detailed implementation
     * @param[in] s ostream to write to
     * @param[in] seat Seat to write out
     * @return modified ostream
     */
    friend std::ostream& operator<<(std::ostream& s, const std::unique_ptr<framework::Bus::Seat>& seat) {
      seat->stream(s);
      return s;
    }

  };  // Seat

 private:
  /**
   * A bus passenger
   *
   * We store the bus passenger's baggage as a dynamically created object.
   * This is necessary so that we can interface with the ROOT TTree.
   *
   * Here we do all the heavy lifting of carrying a certain type
   * of object with specializations for clearing, sorting, and printing.
   * @note Printing specialization is possible if we include the requirement
   * that all event objects must define the 'operator<<' method.
   *
   * ## Core Functions
   *  1. Define the type of "baggage" that is being carried on the event bus
   *  2. "Attach" the baggage to a output or input TTree for writing or reading
   *  3. "Update" the contents of the baggage
   *  4. "Clear" the baggage to a default or empty state at the end of each event
   *  5. "Print" the baggage to a ostream (via the operator<<)
   *
   * ## Basic Structure
   *
   * The method of specializing the clear, post_update, and attach methods
   * to the different types of passenger's baggage is derived from
   * <a
   * href="https://www.fluentcpp.com/2017/08/15/function-templates-partial-specialization-cpp/"
   * >a helpful blog post</a>. 
   * Basically, the idea is to add a parameter input into
   * the functions that "stores" the type that we are currently carrying. This
   * allows the compiler to choose the best method from the overloaded functions
   * by matching the types of parameters.
   *
   * ## Possible Types
   *  - BSILFD - bool, short, int, long, float, double
   *  - class with Clear() method defined
   *  - std::vector of type with operator< defined
   *  - std::map with key type that has operator< defined
   *
   * @tparam[in] BaggageType the type of object that this passenger carries
   */
  template <typename BaggageType>
  class Passenger : public Seat {
   public:
    /**
     * Constructor
     *
     * Dynamically create a default constructed instance of our baggage.
     *
     * @note This requires that all of the BaggageTypes are
     * default-constructible. This is a simple requirement because
     * ROOT dictionary generation already requires a default constructor
     * to be defined.
     */
    Passenger() : Seat(), baggage_{new BaggageType} {}

    /**
     * Destructor
     *
     * Clean up after ourselves by deleting the object
     * we created earlier in the constructor.
     *
     * @note Because the baggage_ member variable
     * may be attached to a TTree, this object needs
     * to be destructed carefully. The TTree assumes
     * that the object we attach to it is valid until
     * we either destruct the TTree or reset its
     * addresses. As such, if we want to destruct a
     * Passenger, we need to know that its baggage
     * is either *not* attached to a TTree or the TTree
     * it was attached to has stopped looking at it
     * (either by deletion or reset).
     */
    virtual ~Passenger() { delete baggage_; }

    /**
     * Attach this passenger to the input tree.
     *
     * Now we know what type of object this passenger
     * is carrying, so we can attach it to the tree.
     *
     * If the 'can_create' parameter is true and a branch of the
     * input name doesn't exist on the tree, we create a new branch
     * of the input branch name. If this parameter is false,
     * we simply return the nullptr signifying that this branch
     * doesn't exist.
     *
     * @see attach(the_type<T>,TTree*,const std::string&,bool)
     * for how we attach to higher-level classes
     *
     * @see attachBasic(TTree*,const std::string&,bool)
     * for how we attach to basic types
     *
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    virtual TBranch* attach(TTree* tree, const std::string& branch_name,
                            bool can_create) {
      return attach(the_type<BaggageType>{}, tree, branch_name, can_create);
    }

    /**
     * Get the object this passenger is carrying
     * @return const reference to the object
     */
    const BaggageType& get() const { return *baggage_; }

    /**
     * Update this passenger's baggage.
     *
     * @see post_update
     * to allow the different types to take actions after the
     * baggage is updated. For example, the vector specialization
     * of post_update uses this opportunity to sort the
     * list of objects.
     *
     * @param[in] updated_obj BaggageType to copy into our object
     */
    void update(const BaggageType& updated_obj) {
      *baggage_ = updated_obj;
      post_update(the_type<BaggageType>());
    }

    /**
     * Reset the object we are carrying to an undefined state.
     *
     * We call the overloaded, templated clear method that
     * allows the compiler to deduce which implementation
     * to use depending on the type of baggage we are carrying.
     */
    virtual void clear() { clear(the_type<BaggageType>{}); }

    /**
     * Stream the passenger's object to the input ostream
     *
     * We call the overloaded, templated stream method that
     * allows the compiler to deduce which implementation
     * to use depending on the type of baggage we are carrying.
     */
    virtual void stream(std::ostream& s) const { 
      stream(the_type<BaggageType>{}, s);
    }

    /**
     * Stream this object to the output stream
     *
     * May contain newlines if large object.
     *
     * @see stream for detailed implementation
     * @param[in] s ostream to write to
     * @param[in] p Passenger to write out
     * @return modified ostream
     */
    friend std::ostream& operator<<(std::ostream& s, const framework::Bus::Passenger<BaggageType>& p) {
      p.stream(s);
      return s;
    }

   private:
    /**
     * A simple, empty struct to allow us to pass the type
     * of baggage to functions as a parameter.
     */
    template <typename>
    struct the_type {};

   private:  // specializations of attach
    /**
     * Attach to a (potentially) new branch on the input tree.
     *
     * This is the attachment mechanism used for all non-basic types.
     * The specializations for the basic types
     * (BSILFD - bool, short, int, long, float, double)
     * use attachBasic which is slightly different in its branching.
     *
     * @see TBranchElement::SetObject
     * for attaching an object to a branch that already exists
     *
     * @see TTree::Branch
     * for creating a new branch
     *
     * With complicated objects like most of our event bus baggage,
     * ROOT is able to deduce the type as long as we load
     * a ROOT dictionary with all of the classes defined inside of it.
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    template <typename T>
    TBranch* attach(the_type<T> t, TTree* tree, const std::string& branch_name,
                    bool can_create) {
      TBranch* branch = tree->GetBranch(branch_name.c_str());
      if (branch) {
        /**
         * If the branch already exists, we need to explicitly remind ROOT
         * that we own the object we are going to pass and it shouldn't
         * try to delete it. Then we set the branch object to our baggage_.
         *
         * Idk why this is necessary. The docs say that you can tell ROOT
         * you own an object by using a non-null pointer passed to SetObject
         * (or its descendent SetAddress); however, in the implementation
         * of TBranchElement::SetAddressImpl, the kDeleteObject status bit
         * is merely tested and the validity of the passed reference is 
         * never checked. This means the bits value is leftover from however
         * the branch was initialized which when reading is ROOT-owned objects.
         */
        if (dynamic_cast<TBranchElement*>(branch)) {
          branch->SetBit(TBranchElement::EStatusBits::kDeleteObject, false);
        }
        branch->SetObject(baggage_);
      } else if (can_create) {
        /**
         * If the branch doesn't already exist and we are allowed to make
         * one, we make a new one passing our baggage.
         */
        branch = tree->Branch(branch_name.c_str(), baggage_, 100000, 3);
      }
      return branch;
    }

    /**
     * Attach a basic type
     *
     * We assume that the current baggage is one of BSILFD
     *    bool, short, int, long, float, double
     *
     * We need this attaching mechanism to be different from the higher-level
     * objects because TTree requires different inputs and returns different
     * outputs for the basic types.
     *
     * @see TBranch::SetAddress
     * for attaching a basic type to a branch that already exists
     *
     * @see TTree::Branch
     * for creating a new branch
     *
     * @see typeid
     * for identifying the type of an object
     *
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    TBranch* attachBasic(TTree* tree, const std::string& branch_name,
                         bool can_create) {
      TBranch* branch = tree->GetBranch(branch_name.c_str());
      if (branch) {
        // branch already exists
        //  set the object the branch should read/write from/to
        branch->SetAddress(baggage_);
      } else if (can_create) {
        static const std::map<std::string, std::string> cpp_to_root_type_name =
            {{"b", "O"}, {"s", "S"}, {"i", "I"},
             {"l", "L"}, {"f", "F"}, {"d", "D"}};
        // branch doesnt exist and we are allowed to make a new one
        std::string cpp_type = typeid(*baggage_).name();
        branch = tree->Branch(
            branch_name.c_str(), baggage_,
            (branch_name + "/" + cpp_to_root_type_name.at(cpp_type)).c_str());
      }
      return branch;
    }

    /**
     * Specialization for bools
     *
     * @see attachBasic for the actual implementation
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    TBranch* attach(the_type<bool> t, TTree* tree,
                    const std::string& branch_name, bool can_create) {
      return attachBasic(tree, branch_name, can_create);
    }

    /**
     * Specialization for shorts
     *
     * @see attachBasic for the actual implementation
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    TBranch* attach(the_type<short> t, TTree* tree,
                    const std::string& branch_name, bool can_create) {
      return attachBasic(tree, branch_name, can_create);
    }

    /**
     * Specialization for ints
     *
     * @see attachBasic for the actual implementation
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    TBranch* attach(the_type<int> t, TTree* tree,
                    const std::string& branch_name, bool can_create) {
      return attachBasic(tree, branch_name, can_create);
    }

    /**
     * Specialization for longs
     *
     * @see attachBasic for the actual implementation
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    TBranch* attach(the_type<long> t, TTree* tree,
                    const std::string& branch_name, bool can_create) {
      return attachBasic(tree, branch_name, can_create);
    }

    /**
     * Specialization for floats
     *
     * @see attachBasic for the actual implementation
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    TBranch* attach(the_type<float> t, TTree* tree,
                    const std::string& branch_name, bool can_create) {
      return attachBasic(tree, branch_name, can_create);
    }

    /**
     * Specialization for doubles
     *
     * @see attachBasic for the actual implementation
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param[in] tree pointer to TTree to attach to
     * @param[in] branch_name name of branch we should attach to
     * @param[in] can_create allow us to create a branch on tree if needed
     * @returns pointer to branch that we attached to (maybe be null)
     */
    TBranch* attach(the_type<double> t, TTree* tree,
                    const std::string& branch_name, bool can_create) {
      return attachBasic(tree, branch_name, can_create);
    }

   private:  // specializations of clear
    /**
     * Clear bool by setting it to false.
     * @param t Unused, only helping compiler choose the correct method
     */
    void clear(the_type<bool> t) { *baggage_ = false; }

    /**
     * Clear short by setting it to the minimum defined by the compiler.
     * @param t Unused, only helping compiler choose the correct method
     */
    void clear(the_type<short> t) {
      *baggage_ = std::numeric_limits<BaggageType>::min();
    }

    /**
     * Clear int by setting it to the minimum defined by the compiler.
     * @param t Unused, only helping compiler choose the correct method
     */
    void clear(the_type<int> t) {
      *baggage_ = std::numeric_limits<BaggageType>::min();
    }

    /**
     * Clear long by setting it to the minimum defined by the compiler.
     * @param t Unused, only helping compiler choose the correct method
     */
    void clear(the_type<long> t) {
      *baggage_ = std::numeric_limits<BaggageType>::min();
    }

    /**
     * Clear float by setting it to the minimum defined by the compiler.
     * @param t Unused, only helping compiler choose the correct method
     */
    void clear(the_type<float> t) {
      *baggage_ = std::numeric_limits<BaggageType>::min();
    }

    /**
     * Clear double by setting it to the minimum defined by the compiler.
     * @param t Unused, only helping compiler choose the correct method
     */
    void clear(the_type<double> t) {
      *baggage_ = std::numeric_limits<BaggageType>::min();
    }

    /**
     * Clear a general class by calling its 'Clear' method.
     *
     * @note The capitalization of 'Clear' is a relic of
     * when the event bus objects needed to inherit from TObject.
     * We _could_ simplify this code by renaming the 'Clear' method
     * to 'clear' which would match std::vector and std::map.
     *
     * @param t Unused, only helping compiler choose the correct method
     */
    template <typename T>
    void clear(the_type<T> t) { baggage_->Clear(); }

    /**
     * Clear a vector by calling the std::vector::clear method.
     * @param t Unused, only helping compiler choose the correct method
     */
    template <typename Content>
    void clear(the_type<std::vector<Content>> t) { baggage_->clear(); }

    /**
     * Clear a map by calling the std::map::clear method.
     * @param t Unused, only helping compiler choose the correct method
     */
    template <typename Key, typename Val>
    void clear(the_type<std::map<Key, Val>> t) { baggage_->clear(); }

   private:  // specializations of post_update
    /**
     * In general, don't do anything after an object has been updated.
     * @param t Unused, only helping compiler choose the correct method
     */
    template <typename T>
    void post_update(the_type<T> t) {}

    /**
     * For std::vector, use the sort method after the contents are updated.
     *
     * @note This is where we require that any contents
     * of vectors have the operator< defined.
     *
     * @param t Unused, only helping compiler choose the correct method
    template <typename Content>
    void post_update(the_type<std::vector<Content>> t) {
      std::sort(baggage_->begin(), baggage_->end());
    }
     */

   private: //specializations of stream
    /**
     * Stream a basic type that has its own
     * definition of 'operator<<'.
     *
     * @note Here is where we require that all event bus
     * objects (not inside a vector or map) need the
     * 'operatore<<' defined.
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param s ostream to write to
     */
    template <typename T>
    void stream(the_type<T> t, std::ostream& s) const {
      //s << *baggagage_;
    }

    /**
     * Stream a vector of objects by looping through them.
     *
     * @note Here is where we require that all event
     * objects inside a vector need the 'operator<<' defined.
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param s ostream to write to
     */
    template <typename Content>
    void stream(the_type<std::vector<Content>> t, std::ostream& s) const {
      s << baggage_->size();
      /*
      s << "[ ";
      for (auto const& entry : *baggage_) s << entry << " ";
      s << "]";
      */
    }

    /**
     * Stream a map of objects by looping through them
     *
     * @note Here is where we require that all event
     * objects used as a key or value in a map
     * need the 'operator<<' defined.
     *
     * @param t Unused, only helping compiler choose the correct method
     * @param s ostream to write to
     */
    template <typename Key, typename Val>
    void stream(the_type<std::map<Key,Val>> t, std::ostream& s) const {
      s << baggage_->size();
      /*
      s << "{ ";
      for (auto const& [k, v] : *baggage_) {
        s << k << " -> " << v << " ";
      }
      s << "}";
      */
    }

   private:
    /// A pointer to the baggage we own and created
    BaggageType* baggage_;
  };  // Passenger


 private:
  /**
   * Get a reference to a passenger on the bus
   *
   * @throws std::bad_cast if BaggageType does not match actual
   * type of object the passenger is carrying.
   *
   * @note Since we are casting from a non-templated base class
   * to a templated derived class, the exception thrown has no
   * knowledge of why the types don't match. (i.e. There **won't**
   * be an exception like 'can't convert float to int'.)
   *
   * @tparam[in] BaggageType type of object passenger is carrying
   * @param[in] name Name of passenger
   * @return reference to passenger
   */
  template <typename BaggageType>
  Passenger<BaggageType>& getRef(const std::string& name) {
    return dynamic_cast<Passenger<BaggageType>&>(*passengers_[name]);
  }

 private:
  /**
   * Map of passenger names to the seats filled by passengers
   *
   * The passenger names are assumed to correspond to any
   * branch name that the passenger's baggage might be attached to.
   */
  std::unordered_map<std::string, std::unique_ptr<Seat>> passengers_;

};  // Bus

}  // namespace framework

#endif  // FRAMEWORK_BUS_H
