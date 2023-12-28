
#ifndef _NTUPLE_MANAGER_H_
#define _NTUPLE_MANAGER_H_

/*~~~~~~~~~~~~*/
/*   StdLib   */
/*~~~~~~~~~~~~*/
#include <map>
#include <string>
#include <unordered_map>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TTree.h"

/*~~~~~~~~~~*/
/*   Core   */
/*~~~~~~~~~~*/
#include "Framework/Bus.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Logger.h"

namespace framework {

/**
 * @class NtupleManager
 * @brief Singleton class used to manage the creation and pooling of
 *        ntuples.
 *
 * @see framework::Bus
 * Similar to the Event bus itself, we use the Bus to buffer the variable
 * values and attach them to their output TTrees. Unlike the event bus,
 * we don't implement retrieval mechanisms for reading these values.
 */
class NtupleManager {
 public:
  /// @return The NtupleManager instance
  static NtupleManager& getInstance();

  /**
   * Create a ROOT tree to hold the ntuple variables (ROOT leaves).
   * @param name Name of the tree.
   */
  void create(const std::string& tname);

  /**
   * Add a variable of type VarType to the ROOT tree with name 'tname'.
   * If the variable already exists in any tree or the requested
   * tree does not exists, an exception is thrown.
   *
   * @tparam[in] VarType type of variable to add to the tree
   * @param[in] tname Name of the tree to add the variable to.
   * @param[in] vname Name of the variable to add to the tree
   * @throws Exception if tree doesn't exist or variable already does
   */
  template <typename VarType>
  void addVar(const std::string& tname, const std::string& vname) {
    // Check if a tree named 'tname' has already been created.  If
    // not, throw an exception.
    if (trees_.count(tname) == 0)
      EXCEPTION_RAISE("NtupleManager", "A tree with name " + tname +
                                           " has not been been created.");

    // Check if the variable exists in the map. If it does, throw
    // an exception.
    if (bus_.isOnBoard(vname)) {
      EXCEPTION_RAISE("NtupleManager", "A variable with name " + vname +
                                           " has already been defined.");
    }

    // Board the bus
    bus_.board<VarType>(vname);

    // Attach the tree to the bus
    bus_.attach(trees_[tname], vname, true);
  }

  /**
   * Set the value of the variable named 'vname'.  If the variable
   * value is not set, the default value will be used when filling
   * the tree.  If the requested variable has not been created,
   * a warning will be printed.  This allows a user to choose to
   * make a subset of an ntuple by simply not adding the variable
   * to the tree.
   *
   * @tparam[in] T type of variable
   * @param[in] vname Name of the variable
   * @param[in] value The value of the variable
   */
  template <typename T>
  void setVar(const std::string& vname, const T& value) {
    // Check if the variable already exists in the map.  If it
    // doesn't, warn the user and don't try to set the variable
    // value.
    if (not bus_.isOnBoard(vname)) {
      ldmx_log(warn) << "The variable " << vname
                     << " does not exist in the tree. Skipping.";
      return;
    }

    // Set the value of the variable
    try {
      bus_.update(vname, value);
    } catch (const std::bad_cast&) {
      EXCEPTION_RAISE("TypeMismatch", "Ntuple variable '" + vname +
                                          "' is being set by the wrong type '" +
                                          typeid(value).name() + "'.");
    }
  }

  // Fill all of the ROOT trees.
  void fill();

  /// Reset all of the variables to their limits.
  void clear();

  /**
   * Reset NtupleManager to blank state
   *
   * We assume that ROOT handles cleanup of TTrees
   * when writing them to a blank state.
   */
  void reset();

  /// Hide Copy Constructor
  NtupleManager(const NtupleManager&) = delete;

  /// Hide Assignment Operator
  void operator=(const NtupleManager&) = delete;

 private:
  /// Container for output ROOT trees
  std::unordered_map<std::string, TTree*> trees_;

  /// Container for buffering variables
  framework::Bus bus_;

  /// Private constructor to prevent instantiation
  NtupleManager();

  /// Enable logging for this singleton
  enableLogging("NtupleManager")

};  // NtupleManager

}  // namespace framework

#endif  // _NTUPLE_MANAGER_H_
