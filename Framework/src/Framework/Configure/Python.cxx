/**
 * @file Python.cxx
 * Implementation of parameter extraction from Python interpreter
 *
 * We need to define a documentation header for this file so that
 * the functions defined within this file are also documented in
 * doxygen.
 */

#include "Framework/Configure/Python.h"

#include "Framework/Exception/Exception.h"

/*~~~~~~~~~~~~*/
/*   python   */
/*~~~~~~~~~~~~*/
#include "Python.h"

#if PY_MAJOR_VERSION != 3
#error("Framework requires compiling with Python3")
#endif

#undef DEV_IMAGE_MAJOR
#if PY_MINOR_VERSION == 6
#define DEV_IMAGE_MAJOR 3
#elif PY_MINOR_VERSION == 10
#define DEV_IMAGE_MAJOR 4
#elif PY_MINOR_VERSION == 12
#define DEV_IMAGE_MAJOR 5
#endif

#ifndef DEV_IMAGE_MAJOR
#warning("Unrecognized Python3 minor version. The usage of the Python C API is untested!")
#endif

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace framework::config {

/**
 * Turn the input python string object into a std::string.
 *
 * Helpful to condense down the multi-line nature of
 * the python3 code.
 *
 * @param[in] pyObj python object assumed to be a string python object
 * @return the value stored in it
 */
static std::string getPyString(PyObject* pyObj) {
  std::string retval;
  PyObject* py_str = PyUnicode_AsEncodedString(pyObj, "utf-8", "Error ~");
  retval = PyBytes_AS_STRING(pyStr);
  Py_XDECREF(pyStr);
  return retval;
}

/**
 * Get a C++ string representation of the input python object
 *
 * This is replicating the `repr(obj)` syntax of Python.
 *
 * @param[in] obj python object to get repr for
 * @return std::string form of repr
 */
std::string repr(PyObject* obj) {
  PyObject* py_repr = PyObject_Repr(obj);
  if (py_repr == nullptr) return "";
  std::string str = getPyString(py_repr);
  Py_XDECREF(py_repr);
  return str;
}

/**
 * extract the dictionary of attributes from the input python object
 *
 * This is separated into its own function to isolate the code
 * that depends on the python version. Since memory-saving measures
 * were integrated into Python between 3.6.9 and 3.10.6, we need to
 * alter how we were using the C API to access an objects __dict__
 * attribute. We are now using a "hidden" Python C API function
 * which has only been added to the public C API documentation
 * in Python 3.11. It has been manually tested for the two different
 * Python versions we use by running the tests (some of which test
 * the ConfigurePython) and running a basic simulation.
 *
 * @throws Exception if object does not have a __dict__ and isn't a dict
 * @param obj pointer to python object to extract from
 * @return pointer to python dictionary for its members
 */
PyObject* extractDictionary(PyObject* obj) {
  /**
   * This was developed for Python3.10 when upgrading to Ubuntu 22.04 in the
   * development container image. A lot of memory-saving measures were taken
   * which means we have to explicitly ask Python to construct the __dict__
   * object for us so that it knows to "waste" the memory on it.
   *
   * https://docs.python.org/3/c-api/object.html
   *
   * We use _PyObject_GetDictPtr because that will not set an error if the
   * dict does not exist.
   */
  PyObject** p_dictionary{_PyObject_GetDictPtr(obj)};
  if (p_dictionary == NULL) {
    if (PyDict_Check(obj)) {
      return obj;
    } else {
      EXCEPTION_RAISE("ObjFail",
                      "Python Object '" + repr(obj) +
                          "' does not have __dict__ member and is not a dict.");
    }
  }
  return *p_dictionary;
}

/**
 * Extract members from a python object.
 *
 * Iterates through the object's dictionary and translates the objects inside
 * of it into the type-specified C++ equivalents, then puts these
 * objects into an instance of the Parameters class.
 *
 * This function is recursive. If a non-base type is encountered,
 * we pass it back along to this function to translate it's own dictionary.
 *
 * We rely completely on python being awesome. For all higher level class
 * objects, python keeps track of all of its member variables in the member
 * dictionary `__dict__`.
 *
 * No Py_DECREF calls are made because all of the members of an object
 * are borrowed references, meaning that when we destory that object, it handles
 * the other members. We destroy the one Python object owning all of
 * these references at the end of this function.
 *
 * @note Not sure if this is not leaking memory, kinda just trusting
 * the Python / C API docs on this one.
 *
 * @note Empty lists are NOT read in because there is no way for us
 * to know what type should be inside the list. This means list
 * parameters that can be empty need to put in a default empty list value:
 * `{}`.
 *
 * This recursive extraction method is able to handle the following cases.
 * - User-defined classes (via the `__dict__` member) are extracted to
 * Parameters
 * - one-dimensional lists whose entries all have the same type are extracted
 *   to std::vector of the type of the first entry in the list
 * - `dict` objects are extracted to Parameters
 * - Python `str` are extracted to std::string
 * - Python `int` are extracted to C++ `int`
 * - Python `bool` are extracted to C++ `bool`
 * - Python `float` are extracted to C++ `double`
 *
 * Known design flaws include
 * - No support for nested Python lists
 * - Annoying band-aid solution for empty Python lists
 *
 * @param[in] object Python object to get members from
 * @return Mapping between member name and value.
 */
static Parameters getMembers(PyObject* object) {
  PyObject* dictionary{extractDictionary(object)};
  PyObject *key(0), *value(0);
  Py_ssize_t pos = 0;

  Parameters params;

  while (PyDict_Next(dictionary, &pos, &key, &value)) {
    std::string skey{getPyString(key)};

    if (PyLong_Check(value)) {
      if (PyBool_Check(value)) {
        params.add(skey, bool(PyLong_AsLong(value)));
      } else {
        params.add(skey, int(PyLong_AsLong(value)));
      }
    } else if (PyFloat_Check(value)) {
      params.add(skey, PyFloat_AsDouble(value));
    } else if (PyUnicode_Check(value)) {
      params.add(skey, getPyString(value));
    } else if (PyList_Check(value)) {
      // assume everything is same value as first value
      if (PyList_Size(value) > 0) {
        auto vec0{PyList_GetItem(value, 0)};

        if (PyLong_Check(vec0)) {
          std::vector<int> vals;

          for (auto j{0}; j < PyList_Size(value); j++)
            vals.push_back(PyLong_AsLong(PyList_GetItem(value, j)));

          params.add(skey, vals);

        } else if (PyFloat_Check(vec0)) {
          std::vector<double> vals;

          for (auto j{0}; j < PyList_Size(value); j++)
            vals.push_back(PyFloat_AsDouble(PyList_GetItem(value, j)));

          params.add(skey, vals);

        } else if (PyUnicode_Check(vec0)) {
          std::vector<std::string> vals;
          for (Py_ssize_t j = 0; j < PyList_Size(value); j++) {
            PyObject* elem = PyList_GetItem(value, j);
            vals.push_back(getPyString(elem));
          }

          params.add(skey, vals);
        } else if (PyList_Check(vec0)) {
          // a list in a list ??? oof-dah
          if (PyList_Size(vec0) > 0) {
            auto vecvec0{PyList_GetItem(vec0, 0)};
            if (PyLong_Check(vecvec0)) {
              std::vector<std::vector<int>> vals;
              for (auto j{0}; j < PyList_Size(value); j++) {
                auto subvec{PyList_GetItem(value, j)};
                std::vector<int> subvals;
                for (auto k{0}; k < PyList_Size(subvec); k++) {
                  subvals.push_back(PyLong_AsLong(PyList_GetItem(subvec, k)));
                }
                vals.push_back(subvals);
              }
              params.add(skey, vals);
            } else if (PyFloat_Check(vecvec0)) {
              std::vector<std::vector<double>> vals;
              for (auto j{0}; j < PyList_Size(value); j++) {
                auto subvec{PyList_GetItem(value, j)};
                std::vector<double> subvals;
                for (auto k{0}; k < PyList_Size(subvec); k++) {
                  subvals.push_back(
                      PyFloat_AsDouble(PyList_GetItem(subvec, k)));
                }
                vals.push_back(subvals);
              }
              params.add(skey, vals);
            } else if (PyUnicode_Check(vecvec0)) {
              std::vector<std::vector<std::string>> vals;
              for (auto j{0}; j < PyList_Size(value); j++) {
                auto subvec{PyList_GetItem(value, j)};
                std::vector<std::string> subvals;
                for (auto k{0}; k < PyList_Size(subvec); k++) {
                  subvals.push_back(getPyString(PyList_GetItem(subvec, k)));
                }
                vals.push_back(subvals);
              }
              params.add(skey, vals);
            } else if (PyList_Check(vecvec0)) {
              EXCEPTION_RAISE("BadConf",
                              "A python list with dimension greater than 2 is "
                              "not supported.");
            } else {
              // RECURSION zoinks!
              std::vector<std::vector<framework::config::Parameters>> vals;
              for (auto j{0}; j < PyList_Size(value); j++) {
                auto subvec{PyList_GetItem(value, j)};
                std::vector<framework::config::Parameters> subvals;
                for (auto k{0}; k < PyList_Size(subvec); k++) {
                  subvals.emplace_back(getMembers(PyList_GetItem(subvec, k)));
                }
                vals.push_back(subvals);
              }
              params.add(skey, vals);
            }
          }  // non-zero size
        } else {
          // RECURSION zoinks!
          // If the objects stored in the list doesn't
          // satisfy any of the above conditions, just
          // create a vector of parameters objects
          std::vector<framework::config::Parameters> vals;
          for (auto j{0}; j < PyList_Size(value); ++j) {
            auto elem{PyList_GetItem(value, j)};
            vals.emplace_back(getMembers(elem));
          }
          params.add(skey, vals);
        }  // type of object in python list
      }  // python list has non-zero size
    } else {
      // object got here, so we assume
      // it is a higher level object
      //(same logic as last option for a list)

      // RECURSION zoinks!
      params.add(skey, getMembers(value));
    }  // python object type
  }  // loop through python dictionary

  return params;
}

Parameters run(const std::string& root_object, const std::string& pythonScript,
               char* args[], int nargs) {
  // assumes that nargs >= 0
  //  this is true always because we error out if no python script has been
  //  found

  // load a handle to the config file into memory (and check that it exists)
  std::unique_ptr<FILE, int (*)(FILE*)> fp{fopen(pythonScript.c_str(), "r"),
                                           &fclose};
  if (fp.get() == NULL) {
    EXCEPTION_RAISE("ConfigDNE",
                    "Passed config script '" + pythonScript +
                        "' is not accessible.\n"
                        "    Did you make a typo in the path to the script?\n"
                        "    Are you referencing a directory that is not "
                        "mounted to the container?");
  }

  // python needs the argument list as if you are on the command line
  //  targs = [ script , arg0 , arg1 , ... ] ==> len(targs) = nargs+1
  // the updated Python3.12 (DEV_IMAGE_MAJOR == 5) C API looks to have
  // more helper functions to avoid having to do this ourselves, but
  // I think sharing the same targs between the different Python versions
  // makes the code cleaner
  wchar_t** targs = new wchar_t*[nargs + 1];
  targs[0] = Py_DecodeLocale(pythonScript.c_str(), NULL);
  for (int i = 0; i < nargs; i++) targs[i + 1] = Py_DecodeLocale(args[i], NULL);

#if DEV_IMAGE_MAJOR < 5
  // name our program after the script that is being run
  Py_SetProgramName(targs[0]);

  // start up python interpreter
  Py_Initialize();

  // The third argument to PySys_SetArgvEx tells python to import
  // the args and add the directory of the first argument to
  // the PYTHONPATH
  // This way, the command to import the module just needs to be
  // the name of the python script
  PySys_SetArgvEx(nargs + 1, targs, 1);
#else
  PyStatus status;
  PyConfig config;
  PyConfig_InitPythonConfig(&config);
  // we do not want python to parse our args (we are already doing that)
  config.parse_argv = 0;
  // note to future developers: the embedding docs encourage users to
  // set config.isolated = 1 in order to more securely embed python.
  // we do /not/ want to do this because we want to inherit the
  // external environment of python

  // copy over program name
  status = PyConfig_SetString(&config, &config.program_name, targs[0]);
  if (PyStatus_Exception(status)) {
    PyConfig_Clear(&config);
    Py_ExitStatusException(status);
    EXCEPTION_RAISE("PyConfigInit",
                    "Unable to set the program name in the python config.");
  }
  // copy over updated argument vector
  status = PyConfig_SetArgv(&config, nargs + 1, targs);
  if (PyStatus_Exception(status)) {
    PyConfig_Clear(&config);
    Py_ExitStatusException(status);
    EXCEPTION_RAISE("PyConfigInit",
                    "Unable to set argv for the python config.");
  }
  // read and solidify the configuration
  status = PyConfig_Read(&config);
  if (PyStatus_Exception(status)) {
    PyConfig_Clear(&config);
    Py_ExitStatusException(status);
    EXCEPTION_RAISE("PyConfigInit", "Unable to read the python config.");
  }
  // initialize the python interpreter with our deduced configuration
  status = Py_InitializeFromConfig(&config);
  if (PyStatus_Exception(status)) {
    PyConfig_Clear(&config);
    Py_ExitStatusException(status);
    Py_FinalizeEx();
    EXCEPTION_RAISE("PyConfigInit",
                    "Unable to initilize the python interpreter.");
  }
  // don't need config anymore now that the initialization is done
  PyConfig_Clear(&config);
#endif

  if (PyRun_SimpleFile(fp.get(), pythonScript.c_str()) != 0) {
    // running the script executed with an error
    PyErr_Print();
    Py_FinalizeEx();
    EXCEPTION_RAISE("Python", "Execution of python script failed.");
  }

  // script has been run so we can
  // free up arguments to python script
  for (int i = 0; i < nargs + 1; i++) PyMem_RawFree(targs[i]);
  delete[] targs;

  // running a python script effectively imports the script into the top-level
  // code environment called '__main__'
  //  we "import" this module which is already imported to get a handle
  //  on the necessary objects
  PyObject* py_root_obj = PyImport_ImportModule("__main__");
  if (!py_root_obj) {
    PyErr_Print();
    Py_FinalizeEx();
    EXCEPTION_RAISE("Python",
                    "I don't know what happened. This should never happen.");
  }

  // descend the hierarchy of modules that hold the root_object
  // manually expanding the '.' allows us to handle all of the different
  // cases of how the configuration Python class could have been imported
  // and constructed
  std::string attr;
  std::stringstream root_obj_ss{root_object};
  while (std::getline(root_obj_ss, attr, '.')) {
    PyObject* one_level_down =
        PyObject_GetAttrString(py_root_obj, attr.c_str());
    if (one_level_down == 0) {
      Py_FinalizeEx();
      EXCEPTION_RAISE("Python", "Unable to find python object '" + attr + "'.");
    }
    Py_DECREF(py_root_obj);  // don't need previous python object anymore
    py_root_obj = one_level_down;
  }

  // now py_root_obj should hold the root configuration object
  if (py_root_obj == Py_None) {
    // root config object left undefined
    Py_FinalizeEx();
    EXCEPTION_RAISE("Python",
                    "Root configuration object " + root_object +
                        " not defined. This object is required to run.");
  }

  // okay, now we have fully imported the script and gotten the handle
  // to the root configuration object defined in the script.
  // We can now look at this object and recursively get all of our parameters
  // out of it.

  Parameters configuration(getMembers(py_root_obj));

  // all done with python nonsense
  // delete one parent python object
  // MEMORY still not sure if this is enough, but not super worried about it
  //  because this only happens once per run
  Py_DECREF(py_root_obj);
  // close up python interpreter
  if (Py_FinalizeEx() < 0) {
    PyErr_Print();
    EXCEPTION_RAISE("Python",
                    "I wasn't able to close up the python interpreter!");
  }

  return configuration;
}

}  // namespace framework::config
