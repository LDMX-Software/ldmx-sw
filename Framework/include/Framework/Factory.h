/**
 * @file Factory.h
 * Header holding Factory class and supporting macros
 */

#ifndef FRAMEWORK_FACTORY_H
#define FRAMEWORK_FACTORY_H

#include <algorithm>                // for for_each call in apply
#include <boost/core/demangle.hpp>  // for demangling
#include <cstdint>
#include <memory>
#include <string>         // for the keys in the library map
#include <unordered_map>  // for the library of prototypes

#include "Framework/Exception/Exception.h"

namespace framework {

/**
 * Factory to dynamically create objects derived from a specific prototype
 * class.
 *
 * This factory should be a singleton class so its copy constructor and
 * assignment operator have been deleted.
 *
 * @tparam Prototype the type of object that this factory creates.
 *    This should be the base class that all types in this factory derive from.
 * @tparam PrototypePtr the type of pointer that the factory creates
 * @tparam PrototypeConstructorArgs parameter pack of arguments to pass
 *    to the object constructor.
 *
 * ## Terminology
 *
 * - Factory: An object that has a look-up table between class names and
 *   pointers to functions that can create them
 * - Maker: A function that can create a specific class
 * - Prototype: An abstract base class from which derived classes can be used
 *
 * ## Design
 *
 * The factory itself works in two steps.
 * 1. All of the different derived classes "declare" themselves
 *    so that the factory knowns how to create them.
 *    This registration is done by providing their type and the
 *    name they should be referred to by.
 *    In order to reduce the amount of boilerplate code, the derived class
 *    declaration can be done with the #FACTORY_REGISTRATION macro.
 * 2. The factory creates any of the registered classes and returns a pointer
 *    to it in the form of a prototype-class pointer.
 *
 * ## Usage
 *
 * Using the factory effectively can be done in situations where many classes
 * all follow the same design structure, but have different implementations
 * for specific steps. In order to reflect this "same design structure",
 * we define an abstract base class for all of our derived classes from
 * which to inherit. This abstract base class is our "prototype".
 *
 * Below is a rudimentary example that shows you the basics of this class.
 *
 * ### A Prototype LibraryEntry
 * This `LibraryEntry` prototype class satisfies our requirements.
 * ```cpp
 * // LibraryEntry.h
 * #ifndef LIBRARYENTRY_H
 * #define LIBRARYENTRY_H
 * // we need the factory template
 * #include "Factory.h"
 *
 * // this class is our prototype
 * class LibraryEntry {
 *  public:
 *   // virtual destructor so we can dynamically create derived classes
 *   virtual ~LibraryEntry() = default;
 *   // pure virtual function that our derived classes will implement
 *   virtual std::string name() = 0;
 *   // the factory type that we will use here
 *   DECLARE_FACTORY(LibraryEntry, std::shared_ptr<LibraryEntry>);
 * };  // LibraryEntry
 *
 * #endif // LIBRARYENTRY_H
 *
 * // LibraryEntry.cxx
 * #include "LibraryEntry.hpp"
 * DEFINE_FACTORY(LibraryEntry);
 * ```
 *
 * ### Example Derived Classes
 * Here are a few example derived classes.
 * ```cpp
 * // Book.cxx
 * #include "LibraryEntry.hpp"
 * namespace library {
 * class Book : public LibraryEntry {
 *  public :
 *   virtual std::string name() override {
 *     return "Where the Red Fern Grows";
 *   }
 * };
 * }
 *
 * FACTORY_REGISTRATION(LibraryEntry, library::Book);
 * ```
 *
 * ```cpp
 * // Podcast.cpp
 * #include "LibraryEntry.hpp"
 * namespace library {
 * namespace audio {
 * class Podcast : public LibraryEntry {
 *  public :
 *   virtual std::string name() override {
 *     return "More Rim and AB";
 *   }
 * };
 * }
 * }
 *
 * FACTORY_REGISTRATION(LibraryEntry, library::audio::Podcast);
 * ```
 *
 * ```cpp
 * // Album.cpp
 * #include "LibraryEntry.hpp"
 * namespace library {
 * namespace audio {
 * class Album : public LibraryEntry {
 *  public :
 *   virtual std::string name() override {
 *     return "Kind of Blue";
 *   }
 * };
 * }
 * }
 *
 * FACTORY_REGISTRATION(LibraryEntry, library::audio::Album);
 * ```
 *
 * ### Executable
 * Since the #FACTORY_REGISTRATION macro defines a function whose result
 * produces a static variable, the function is called at library-load
 * time, the registration of our various library entries is automatically done
 * before the execution of `main`
 * (or after if the libraries are dynamically loaded with dlopen).
 * For simplicity, let's compile these sources files together with a
 * main defined below.
 *
 * ```cpp
 * // main.cxx
 * #include "LibraryEntry.hpp"
 *
 * int main(int argc, char* argv[]) {
 *   std::string full_cpp_name{argv[1]};
 *   auto entry_ptr{LibraryEntry::Factory::get().make(full_cpp_name)};
 *   if (entry_ptr) {
 *     std::cout << entry_ptr->name() << std::endl;
 *   } else {
 *     std::cerr << "ERROR: could not create a " << full_cpp_name << std::endl;
 *   }
 * }
 * ```
 *
 * Compiling these files together into the `fave-things` executable would
 * then lead to the following behavior.
 *
 * ```
 * $ fave-things library::Book
 * Where the Red Fern Grows
 * $ fave-things library::audio::Podcast
 * More Rim and AB
 * $ fave-things library::audio::Album
 * Kind of Blue
 * $ fave-things library::DoesNotExist
 * ERROR: could not create a library::DoesNotExist
 * ```
 */
template <typename Prototype, typename PrototypePtr,
          typename... PrototypeConstructorArgs>
class Factory {
 public:
  /**
   * the signature of a function that can be used by this factory
   * to dynamically create a new object.
   *
   * This is merely here to make the definition of the Factory simpler.
   */
  using PrototypeMaker = PrototypePtr (*)(PrototypeConstructorArgs...);

 public:
  /**
   * register a new object to be constructible
   *
   * We insert the new object into the library **without checking if we
   * are overwriting anything**.
   *
   * @note The derived_type_name argument is what should be used as an input
   * to Factory::make. Most commonly, this is just the fully-specificed C++
   * class name which is what is done with the #FACTORY_REGISTRATION macro, but
   * it could be something else if the user wishes to use this function directly
   * instead of the macro.
   *
   * @tparam DerivedType object type to declare
   * @param[in] derived_type_name name of DerivedType to be used in the mapping
   * @return value to define a static variable to force running this function
   *  at library load time. It relates to variables so that it cannot be
   *  optimized away.
   */
  template <typename DerivedType>
  uint64_t declare(const std::string& derived_type_name) {
    library_[derived_type_name] = &maker<DerivedType>;
    return reinterpret_cast<std::uintptr_t>(&library_);
  }

  /**
   * make a new object by name
   *
   * We look through the library to find the requested object.
   * If found, we create one, store it inside our warehouse_,
   * and return a pointer to the newly created object.
   * If not found, we return a nullptr.
   *
   * The arguments to the maker are determined at compiletime
   * using the template parameters of Factory.
   *
   * @param[in] full_name name of class to create
   * same name as passed to declare
   * @param[in] maker_args parameter pack of arguments to pass on to maker
   *
   * @returns a pointer to the parent class that the objects derive from (or
   * null)
   */
  PrototypePtr make(const std::string& full_name,
                    PrototypeConstructorArgs... maker_args) {
    auto lib_it{library_.find(full_name)};
    if (lib_it == library_.end()) {
      return PrototypePtr(nullptr);
    }
    warehouse_.emplace_back(lib_it->second(maker_args...));
    return warehouse_.back();
  }

  /**
   * Apply the input UnaryFunction to each entry in the warehouse
   *
   * UnaryFunction is simply passed dirctly to std::for_each so
   * look there for requirements upon it.
   *
   * @tparam UnaryFunction type of function to be applied, its return
   * value is ignored and its only argument are PrototypePtr objects.
   * @param[in] f UnaryFunction to apply to each entry
   */
  template <class UnaryFunction>
  void apply(UnaryFunction f) const {
    std::for_each(warehouse_.begin(), warehouse_.end(), f);
  }

  /// delete the copy constructor
  Factory(Factory const&) = delete;

  /// delete the assignment operator
  void operator=(Factory const&) = delete;

  /// default constructor that does nothing
  Factory() = default;

 private:
  /**
   * make a new DerivedType returning a PrototypePtr
   *
   * Basically a copy of what
   * [`std::make_unique`](https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)
   * or
   * [`std::make_shared`](https://en.cppreference.com/w/cpp/memory/shared_ptr/make_shared)
   * do but with the following changes:
   *  1. constructor arguments defined by the Factory and not here
   *  2. return type is a base pointer and not a derived pointer
   *
   * This is where we required that PrototypePtr has the same
   * behavior as STL smart pointers. The PrototypePtr class must
   * be able to be constructed from a pointer to a derived class
   * and must take ownership of the new object.
   *
   * @tparam DerivedType type of derived object we should create
   * @param[in] args constructor arguments for derived type construction
   */
  template <typename DerivedType>
  static PrototypePtr maker(PrototypeConstructorArgs... args) {
    return PrototypePtr(
        new DerivedType(std::forward<PrototypeConstructorArgs>(args)...));
  }

  /// library of possible objects to create
  std::unordered_map<std::string, PrototypeMaker> library_;

  /// warehouse of objects that have already been created
  std::vector<PrototypePtr> warehouse_;
};  // Factory

}  // namespace framework

/**
 * This macro is used in the `public` portion of your prototype class
 * declaration.
 *
 * ```cpp
 * public:
 *  DECLARE_FACTORY(MyProto, std::shared_ptr<MyProto>);
 * ```
 *
 * The arguments to this macro are the template arguments to the
 * framework::Factory class and should at minimum define the base class the
 * Factory will construct for and the type of pointer the Factory should return.
 */
#define DECLARE_FACTORY(...)                                  \
  struct Factory : public ::framework::Factory<__VA_ARGS__> { \
    static Factory& get();                                    \
  }

/**
 * This should go into an implementation file for your prototype class.
 *
 * ```cpp
 * DEFINE_FACTORY(MyProto);
 * ```
 *
 * We need this to be separate from the declaration so that there is
 * a unique single factory for the class across any of the C++ libraries
 * that may use it.
 */
#define DEFINE_FACTORY(classtype)                 \
  classtype::Factory& classtype::Factory::get() { \
    static classtype::Factory the_factory;        \
    return the_factory;                           \
  }

/**
 * Concatenate two pieces of text into one
 *
 * We use the `__COUNTER__` macro to uniquely define variable
 * names within a translation unit (file) but we need to pass
 * `__COUNTER__` through a macro argument in order to have it
 * resolved into a number.
 */
#define _CONCAT_INTERNAL(a, b) a##b

/**
 * Concatenate two pieces of text into one with indirection
 *
 * We use the `__COUNTER__` macro to uniquely define variable
 * names within a translation unit (file) but we need to pass
 * `__COUNTER__` through a macro argument in order to have it
 * resolved into a number.
 */
#define _CONCAT(a, b) _CONCAT_INTERNAL(a, b)

/**
 * Add a number suffix to the input variable name so that it is unique
 *
 * We use the `__COUNTER__` macro which counts up from 0 everytime it
 * is called so these variables can be unique within a specific
 * translation unit.
 */
#define UNIQUE(a) _CONCAT(a, __COUNTER__)

/**
 * Register a new class with a specific factory
 *
 * This macro should be used where the derived class is defined.
 * This macro avoids typing all this out and making sure that the string
 * passed as an argument to framework::Factory::declare is the same characters as
 * the actual class.
 * If you want the name for a specific object to not be the full-specified
 * class name, then you need to write the contents of this macro yourself.
 *
 * Using an [unnamed namespace](
 * https://en.cppreference.com/w/cpp/language/namespace#Unnamed_namespaces)
 * defines the variables inside it as having internal linkage and as implicitly
 * static. Having internal linkage allows us to have repeat variable names
 * across different source files. Being static means that the variable is
 * guaranteed to be constructed during library load time.
 * The details of how this is handled is documented in
 * [Storage Class Specifiers](
 * https://en.cppreference.com/w/cpp/language/storage_duration).
 *
 * @param[in] prototype fully-specified base class
 * @param[in] derived fully-specified derived class
 */
#define FACTORY_REGISTRATION(prototype, derived)                             \
  namespace {                                                                \
  auto UNIQUE(v) = ::prototype::Factory::get().declare<::derived>(#derived); \
  }

#endif
