
#include "Framework/ConfigurePython.h"

/*~~~~~~~~~~~~*/
/*   python   */
/*~~~~~~~~~~~~*/
#include "Python.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace framework {

std::string ConfigurePython::root_module = "ldmxcfg";
std::string ConfigurePython::root_class = "Process";
std::string ConfigurePython::root_object = "lastProcess";

/**
 * Turn the input python string object into a C++ string.
 *
 * Helpful to condense down the multi-line nature of
 * the python3 code.
 *
 * @param[in] python object assumed to be a string python object
 * @return the value stored in it
 */
static std::string getPyString(PyObject* pyObj) {
  std::string retval;
  PyObject* pyStr = PyUnicode_AsEncodedString(pyObj, "utf-8", "Error ~");
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
#if PY_MAJOR_VERSION != 3
#error("Framework requires compiling with Python3")
#else
#if PY_MINOR_VERSION != 10 && PY_MINOR_VERSION != 6
#warning("Unrecognized Python3 minor version. Unsure if accessing C API properly for configuration.")
#endif
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
#endif
}

/**
 * Extract members from a python object.
 *
 * Iterates through the object's dictionary and translates the objects inside
 * of it into the type-specified C++ equivalents, then puts these
 * objects into a STL map that can be passed to the Parameters class.
 *
 * This function is recursive. If a non-base type is encountered,
 * we pass it back along to this function to translate it's own dictionary.
 *
 * We rely completely on python being awesome. For all higher level class
 * objects, python keeps track of all of its member variables in the member
 * dictionary __dict__.
 *
 * No Py_DECREF calls are made because all of the members of an object
 * are borrowed references, meaning that when we destory that object, it handles
 * the other members. We destroy the one parent object pProcess at the end
 * of ConfigurePython::ConfigurePython.
 *
 * @note Not sure if this is not leaking memory, kinda just trusting
 * the Python / C API docs on this one.
 *
 * @note Empty lists are NOT read in because there is no way for us
 * to know what type should be inside the list. This means list
 * parameters that can be empty need to put in a default empty list
 * value: {}.
 *
 * @param object Python object to get members from
 * @return Mapping between member name and value.
 */
static std::map<std::string, std::any> getMembers(PyObject* object) {
  PyObject* dictionary{extractDictionary(object)};
  PyObject *key(0), *value(0);
  Py_ssize_t pos = 0;

  std::map<std::string, std::any> params;

  while (PyDict_Next(dictionary, &pos, &key, &value)) {
    std::string skey{getPyString(key)};
    if (PyLong_Check(value)) {
      if (PyBool_Check(value)) {
        params[skey] = bool(PyLong_AsLong(value));
      } else {
        params[skey] = int(PyLong_AsLong(value));
      }
    } else if (PyFloat_Check(value)) {
      params[skey] = PyFloat_AsDouble(value);
    } else if (PyUnicode_Check(value)) {
      params[skey] = getPyString(value);
    } else if (PyList_Check(value)) {
      // assume everything is same value as first value
      if (PyList_Size(value) > 0) {
        auto vec0{PyList_GetItem(value, 0)};

        if (PyLong_Check(vec0)) {
          std::vector<int> vals;

          for (auto j{0}; j < PyList_Size(value); j++)
            vals.push_back(PyLong_AsLong(PyList_GetItem(value, j)));

          params[skey] = vals;

        } else if (PyFloat_Check(vec0)) {
          std::vector<double> vals;

          for (auto j{0}; j < PyList_Size(value); j++)
            vals.push_back(PyFloat_AsDouble(PyList_GetItem(value, j)));

          params[skey] = vals;

        } else if (PyUnicode_Check(vec0)) {
          std::vector<std::string> vals;
          for (Py_ssize_t j = 0; j < PyList_Size(value); j++) {
            PyObject* elem = PyList_GetItem(value, j);
            vals.push_back(getPyString(elem));
          }

          params[skey] = vals;
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
              params[skey] = vals;
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
              params[skey] = vals;
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
              params[skey] = vals;
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
                  subvals.emplace_back();
                  subvals.back().setParameters(
                      getMembers(PyList_GetItem(subvec, k)));
                }
                vals.push_back(subvals);
              }
              params[skey] = vals;
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
            vals.emplace_back();
            vals.back().setParameters(getMembers(elem));
          }
          params[skey] = vals;
        }  // type of object in python list
      }    // python list has non-zero size
    } else {
      // object got here, so we assume
      // it is a higher level object
      //(same logic as last option for a list)

      // RECURSION zoinks!
      framework::config::Parameters val;
      val.setParameters(getMembers(value));

      params[skey] = val;

    }  // python object type
  }    // loop through python dictionary

  return std::move(params);
}

ConfigurePython::ConfigurePython(const std::string& pythonScript, char* args[],
                                 int nargs) {
  // assumes that nargs >= 0
  //  this is true always because we error out if no python script has been
  //  found

  std::string cmd = pythonScript;
  if (pythonScript.rfind("/") != std::string::npos) {
    cmd = pythonScript.substr(pythonScript.rfind("/") + 1);
  }
  cmd = cmd.substr(0, cmd.find(".py"));

  // python needs the argument list as if you are on the command line
  //  targs = [ script , arg0 , arg1 , ... ] ==> len(targs) = nargs+1
  // PySys_SetArgvEx uses wchar_t instead of char in python3
  wchar_t** targs = new wchar_t*[nargs + 1];
  targs[0] = Py_DecodeLocale(pythonScript.c_str(), NULL);
  for (int i = 0; i < nargs; i++) targs[i + 1] = Py_DecodeLocale(args[i], NULL);

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

  // the following line is what actually runs the script
  PyObject* script = PyImport_ImportModule(cmd.c_str());

  // script has been run so we can
  // free up arguments to python script
  for (int i = 0; i < nargs + 1; i++) PyMem_RawFree(targs[i]);
  delete[] targs;

  if (script == 0) {
    PyErr_Print();
    EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
  }

  PyObject* pCMod = PyObject_GetAttrString(script, root_module.c_str());
  Py_DECREF(script);  // don't need the script anymore
  if (pCMod == 0) {
    PyErr_Print();
    EXCEPTION_RAISE("ConfigureError",
                    "Did not import root configuration module " + root_module);
  }

  PyObject* pProcessClass = PyObject_GetAttrString(pCMod, root_class.c_str());
  Py_DECREF(pCMod);  // don't need the config module anymore
  if (pProcessClass == 0) {
    PyErr_Print();
    EXCEPTION_RAISE("ConfigureError",
                    "Did not import root configuration class " + root_class);
  }

  PyObject* pProcess =
      PyObject_GetAttrString(pProcessClass, root_object.c_str());
  Py_DECREF(pProcessClass);  // don't need the Process class anymore
  if (pProcess == 0) {
    // wasn't able to get lastProcess class member
    PyErr_Print();
    EXCEPTION_RAISE(
        "ConfigureError",
        "Process object not defined. This object is required to run.");
  } else if (pProcess == Py_None) {
    // lastProcess was left undefined
    EXCEPTION_RAISE("ConfigureError",
                    "Did not create a configuration class instance");
  }

  // okay, now we have fully imported the script and gotten the handle
  // to the last Process object defined in the script.
  // We can now look at pProcess and get all of our parameters out of it.

  configuration_.setParameters(getMembers(pProcess));

  // all done with python nonsense
  // delete one parent python object
  // MEMORY still not sure if this is enough, but not super worried about it
  //  because this only happens once per run
  Py_DECREF(pProcess);
  // close up python interpreter
  if (Py_FinalizeEx() < 0) {
    PyErr_Print();
    EXCEPTION_RAISE("PyError",
                    "I wasn't able to close up the python interpreter!");
  }
}

ProcessHandle ConfigurePython::makeProcess() {
  // no python nonsense happens in here,
  // this just takes the parameters determined earlier
  // and puts them into the Process + EventProcessor framework

  return std::make_unique<Process>(configuration_);
}

}  // namespace framework
