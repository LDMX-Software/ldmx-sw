#ifndef FRAMEWORK_PARAMETERS_H
#define FRAMEWORK_PARAMETERS_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include <boost/core/demangle.hpp>
#include "Framework/Exception/Exception.h"

namespace framework::config {

/**
 * Class encapsulating parameters for configuring a processor.
 *
 * The storage of arbitrary parameters recursively is done using a map
 * from parameter names (std::string) to parameter values. The values
 * are stored in std::any which allows the user to store any object they wish 
 * in a memory-safe environment.
 */
class Parameters {
 public:
  /**
   * Add a parameter to the parameter list.  If the parameter already
   * exists in the list, throw an exception.
   *
   * @param[in] name Name of the parameter.
   * @param[in] value The value of the parameter.
   * @tparam T type of parameter being added
   * @throw Exception if a parameter by that name already exist in
   *  the list.
   */
  template <typename T>
  void add(const std::string& name, const T& value) {
    if (exists(name)) {
      EXCEPTION_RAISE("Config",
          "The parameter " + name + " already exists in the list of parameters.");
    }

    parameters_[name] = value;
  }

  template <typename T>
  void addParameter(const std::string& name, const T& value) {
    add<T>(name,value);
  }

  /**
   * Check to see if a parameter exists
   *
   * @param[in] name name of parameter to check
   * @return true if parameter exists in configuration set
   */
  bool exists(const std::string& name) const {
    return parameters_.find(name) != parameters_.end();
  }

  /**
   * Retrieve the parameter of the given name.
   *
   * @throw Exception if parameter of the given name isn't found
   * @throw Exception if parameter is found but not of the input type
   *
   * @tparam T the data type to cast the parameter to.
   * @param[in] name the name of the parameter value to retrieve.
   * @return The user specified parameter of type T.
   */
  template <typename T>
  const T& get(const std::string& name) const {
    // Check if the variable exists in the map.  If it doesn't,
    // raise an exception.
    if (not exists(name)) {
      EXCEPTION_RAISE("Config","Parameter '" + name + "' does not exist in list of parameters.");
    }

    try {
      return std::any_cast<const T&>(parameters_.at(name));
    } catch (const std::bad_any_cast& e) {
      EXCEPTION_RAISE("Config","Parameter '" + name + "' of type '" +
                          boost::core::demangle(parameters_.at(name).type().name()) +
                          "' is being cast to incorrect type '" +
                          boost::core::demangle(typeid(T).name()) + "'.");
    }
  }

  template <typename T>
  const T& getParameter(const std::string& name) const {
    return get<T>(name);
  }

  /**
   * Retrieve a parameter with a default specified.
   *
   * Return the input default if a parameter is not found in map.
   *
   * @tparam T type of parameter to return
   * @param[in] name parameter name to retrieve
   * @param[in,out] def default to use if parameter is not found
   * @return the user parameter of type T
   */
  template <typename T>
  const T& get(const std::string& name, const T& def) const {
    if (not exists(name)) return def;

    // get here knowing that name exists in parameters_
    return get<T>(name);
  }

  template <typename T>
  const T& getParameter(const std::string& name, const T& def) const {
    return get<T>(name,def);
  }

  /**
   * Get a list of the keys available.
   *
   * This may be helpful in debugging to make sure the parameters are spelled
   * correctly.
   *
   * @return list of all keys in this map of parameters
   */
  std::vector<std::string> keys() const {
    std::vector<std::string> key;
    for (auto i : parameters_) key.push_back(i.first);
    return key;
  }

 private:
  /// container holding parameter names and their values
  std::map<std::string, std::any> parameters_;

};  // Parameters
}  // namespace framework::config

#endif  // FRAMEWORK_PARAMETERS_H
