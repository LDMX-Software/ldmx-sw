#ifndef SIMCORE_FACTORY_H
#define SIMCORE_FACTORY_H

#include <algorithm>                // for for_each call in apply
#include <boost/core/demangle.hpp>  // for demangling
#include <memory>                   // for the unique_ptr default
#include <string>                   // for the keys in the library map
#include <unordered_map>            // for the library of prototypes

#include "Framework/Exception/Exception.h"

namespace simcore {

/**
 * Factory to dynamically create objects derived from a specific prototype
 * class.
 *
 * This factory is a singleton class meaning it cannot be created by the user.
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
 * 2. The factory creates any of the registered classes and returns a pointer
 *    to it in the form of a prototype-class pointer.
 *
 * ### Declaration
 * Using an
 * [unnamed
 namespace](https://en.cppreference.com/w/cpp/language/namespace#Unnamed_namespaces)
 * defines the variables inside it as having internal linkage and as implicitly
 * static. Having internal linkage allows us to have repeat variable names
 * across different source files. Being static means that the variable is
 * guaranteed to be constructed during library load time.
 *
 * This if we put the following code in the source file for a class deriving
 * from our prototype, it will be declared to the factory during library load.
 * ```cpp
 * // MyDerived.cpp
 * // MyDerived inherits from MyPrototype
 * namespace {
 *   auto v = ::fire::factory::Factory<MyPrototype>::get()
 *     .declare<MyDerived>();
 * }
 * ```
 *
 * The details of how this is handled is documented in
 * [Storage Class
 Specifiers](https://en.cppreference.com/w/cpp/language/storage_duration).
 *
 * ## Usage
 *
 * Using the factory effecitvely can be done in situations where many classes
 * all follow the same design structure, but have different implementations
 * for specific steps. In order to reflect this "same design structure",
 * we define an abstract base class for all of our derived classes from
 * which to inherit. This abstract base class is our "prototype".
 *
 * Below is a rudimentary example that shows you the basics of this class.
 *
 * ### A Prototype LibraryEntry
 * This `LibraryEntry` prototype class satisfies our requirements.
 * It also defines a helpful "declaration" macro for derived classes to use.
 * ```cpp
 * // LibraryEntry.hpp
 * #ifndef LIBRARYENTRY_HPP
 * #define LIBRARYENTRY_HPP
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
 *   using Factory = ::fire::factory::Factory<LibraryEntry>;
 * };  // LibraryEntry
 *
 * // a macro to help with registering our library entries with our factory
 * #define DECLARE_LIBRARYENTRY(CLASS)                          \
 *   namespace {                                                \
 *     auto v = ::LibraryEntry::Factory::get().declare<CLASS>() \
 *   }
 * #endif // LIBRARYENTRY_HPP
 * ```
 *
 * ### Example Derived Classes
 * Here are a few example derived classes.

 * ```cpp
 * // Book.cpp
 * #include "LibraryEntry.hpp"
 * namespace library {
 * class Book : public LibraryEntry {
 *  public :
 *   virtual std::string name() final override {
 *     return "Where the Red Fern Grows";
 *   }
 * };
 * }
 *
 * DECLARE_LIBRARYENTRY(library::Book)
 * ```
 *
 * ```cpp
 * // Podcast.cpp
 * #include "LibraryEntry.hpp"
 * namespace library {
 * namespace audio {
 * class Podcast : public LibraryEntry {
 *  public :
 *   virtual std::string name() final override {
 *     return "538 Politics Podcast";
 *   }
 * };
 * }
 * }
 *
 * DECLARE_LIBRARYENTRY(library::audio::Podcast)
 * ```
 *
 * ```cpp
 * // Album.cpp
 * #include "LibraryEntry.hpp"
 * namespace library {
 * namespace audio {
 * class Album : public LibraryEntry {
 *  public :
 *   virtual std::string name() final override {
 *     return "Kind of Blue";
 *   }
 * };
 * }
 * }
 *
 * DECLARE_LIBRARYENTRY(library::audio::Album)
 * ```
 *
 * ### Executable
 * Since the `DECLARE_LIBRARYENTRY` macro defines a function that is decorated
 * with a compiler attribute causing the function to be called at library-load
 * time, the registration of our various library entries is automatically done
 * before the execution of `main` (or after if the loadLibrary function is
 * used). For simplicity, let's compile these sources files together with a
 * main defined below.
 *
 * ```cpp
 * // main.cxx
 * #include "LibraryEntry.hpp"
 *
 * int main(int argc, char* argv[]) {
 *   std::string full_cpp_name{argv[1]};
 *   try {
 *     auto entry_ptr{LibraryEntry::Factory::get().make(full_cpp_name)};
 *     std::cout << entry_ptr->name() << std::endl;
 *   } catch (const std::exception& e) {
 *     std::cerr << "ERROR: " <<  e.what() << std::endl;
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
 * 538 Politics Podcast
 * $ fave-things library::audio::Album
 * Kind of Blue
 * $ fave-things library::DoesNotExist
 * ERROR: An object named library::DoesNotExist has not been declared.
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
   * get the factory instance
   *
   * Using a static function variable gaurantees that the factory
   * is created as soon as it is needed and that it is deleted
   * before the program completes.
   *
   * @returns reference to single Factory instance
   */
  static Factory& get() {
    static Factory the_factory;
    return the_factory;
  }

  /**
   * register a new object to be constructible
   *
   * We insert the new object into the library after
   * checking that it hasn't been defined before.
   *
   * @note This uses the demangled name of the input type
   * as the key in our library of objects. Using the demangled
   * name effectively assumes that all of the libraries being
   * loaded were compiled with the same compiler version.
   * We could undo this assumption by having the key be an
   * input into this function.
   *
   * @tparam DerivedType object type to declare
   * @return value to define a static variable to force running this function
   *  at library load time. It relates to variables so that it cannot be
   *  optimized away.
   */
  template <typename DerivedType>
  uint64_t declare() {
    std::string full_name{boost::core::demangle(typeid(DerivedType).name())};
    library_[full_name] = &maker<DerivedType>;
    return reinterpret_cast<std::uintptr_t>(&library_);
  }

  /**
   * make a new object by name
   *
   * We look through the library to find the requested object.
   * If found, we create one and return a pointer to the newly
   * created object. If not found, we raise an exception.
   *
   * @throws Exception if the input object name could not be found
   *
   * The arguments to the maker are determined at compiletime
   * using the template parameters of Factory.
   *
   * @param[in] full_name name of class to create, same name as passed to
   * declare
   * @param[in] maker_args parameter pack of arguments to pass on to maker
   *
   * @returns a pointer to the parent class that the objects derive from.
   */
  PrototypePtr make(const std::string& full_name,
                    PrototypeConstructorArgs... maker_args) {
    auto lib_it{library_.find(full_name)};
    if (lib_it == library_.end()) {
      EXCEPTION_RAISE("SimFactory", "An object named " + full_name +
                                        " has not been declared.");
    }
    warehouse_.emplace_back(lib_it->second(maker_args...));
    return warehouse_.back();
  }

  /**
   * Apply the input UnaryFunction to each entry in the inventory
   *
   * UnaryFunction is simply passed dirctly to std::for_each so
   * look there for requirements upon it.
   */
  template <class UnaryFunction>
  void apply(UnaryFunction f) const {
    std::for_each(warehouse_.begin(), warehouse_.end(), f);
  }

  /// delete the copy constructor
  Factory(Factory const&) = delete;

  /// delete the assignment operator
  void operator=(Factory const&) = delete;

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

  /// private constructor to prevent creation
  Factory() = default;

  /// library of possible objects to create
  std::unordered_map<std::string, PrototypeMaker> library_;

  /// warehouse of objects that have already been created
  std::vector<PrototypePtr> warehouse_;
};  // Factory

}  // namespace simcore
#endif  // SIMCORE_FACTORY_H
