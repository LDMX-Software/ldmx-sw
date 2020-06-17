
#include "Framework/ConfigurePython.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventProcessorFactory.h"

/*~~~~~~~~~~~~*/
/*   python   */
/*~~~~~~~~~~~~*/
#include "Python.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>

/*~~~~~~~~~~~~~~~~*/
/*     ROOT       */
/*~~~~~~~~~~~~~~~~*/
#include "TH1F.h" //for creating histograms

namespace ldmx {

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
        PyObject* pyStr = PyUnicode_AsEncodedString(pyObj, "utf-8","Error ~");
        retval = PyBytes_AS_STRING(pyStr);
        Py_XDECREF(pyStr);
        return retval;
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
     * We rely completely on python being awesome. For all higher level class objects,
     * python keeps track of all of its member variables in the member dictionary __dict__.
     *
     * No Py_DECREF calls are made because all of the members of an object
     * are borrowed references, meaning that when we destory that object, it handles
     * the other members. We destroy the one parent object pProcess at the end
     * of ConfigurePython::ConfigurePython.
     *
     * @note Not sure if this is not leaking memory, kinda just trusting
     * the Python / C API docs on this one.
     *
     * @param object Python object to get members from
     * @return Mapping between member name and value. 
     */
    static std::map< std::string, std::any > getMembers(PyObject* object) {
        
        PyObject* dictionary{PyObject_GetAttrString(object, "__dict__")};

        PyObject *key(0), *value(0);
        Py_ssize_t pos = 0;

        std::map < std::string, std::any > params; 

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
            } else if (PyList_Check(value)) { // assume everything is same value as first value
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
                        for (Py_ssize_t j = 0; j < PyList_Size(value); j++){
                            PyObject* elem = PyList_GetItem(value , j );
                            vals.push_back(getPyString(elem));
                        }

                        params[skey] = vals;

                    } else { 

                        // RECURSION zoinks!
                        // If the objects stored in the list doesn't 
                        // satisfy any of the above conditions, just
                        // create a vector of parameters objects
                        std::vector< Parameters > vals;
                        for (auto j{0}; j < PyList_Size(value); ++j) {

                            auto obj{PyList_GetItem( value, j )}; 

                            vals.emplace_back();
                            vals.back().setParameters( getMembers(obj) );

                        } 
                        params[skey] = vals;

                    } //type of object in python list
                } //python list has non-zero size
            } else {
                //object got here, so we assume
                //it is a higher level object 
                //(same logic as last option for a list)

                // RECURSION zoinks!
                Parameters val;
                val.setParameters( getMembers(value) );

                params[skey] = val;

            } //python object type
        } //loop through python dictionary

        return std::move(params); 
    }

    ConfigurePython::ConfigurePython(const std::string& pythonScript, char* args[], int nargs) {

        //assumes that nargs >= 0
        //  this is true always because we error out if no python script has been found

        std::string cmd = pythonScript;
        if (pythonScript.rfind("/") != std::string::npos) {
            cmd = pythonScript.substr(pythonScript.rfind("/") + 1);
        }
        cmd = cmd.substr(0, cmd.find(".py"));
        
        //python needs the argument list as if you are on the command line
        //  targs = [ script , arg0 , arg1 , ... ] ==> len(targs) = nargs+1
        //PySys_SetArgvEx uses wchar_t instead of char in python3
        wchar_t** targs = new wchar_t*[nargs+1];
        targs[0] = Py_DecodeLocale(pythonScript.c_str(), NULL);
        for (int i = 0; i < nargs;  i++)
            targs[i+1] = Py_DecodeLocale(args[i], NULL);

        //name our program after the script that is being run
        Py_SetProgramName(targs[0]);

        //start up python interpreter
        Py_Initialize();

        //The third argument to PySys_SetArgvEx tells python to import
        //the args and add the directory of the first argument to
        //the PYTHONPATH
        //This way, the command to import the module just needs to be
        //the name of the python script
        PySys_SetArgvEx(nargs+1, targs, 1);

        //the following line is what actually runs the script
        PyObject* script = PyImport_ImportModule(cmd.c_str());

        // script has been run so we can
        // free up arguments to python script
        for(int i = 0; i < nargs+1; i++)
            PyMem_RawFree(targs[i]);
        delete [] targs;

        if (script == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
        }

        PyObject* pCMod = PyObject_GetAttrString(script, "ldmxcfg");
        Py_DECREF(script); //don't need the script anymore
        if (pCMod == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
        }

        PyObject* pProcessClass = PyObject_GetAttrString(pCMod, "Process");
        Py_DECREF(pCMod); //don't need the config module anymore
        if (pProcessClass == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", 
                    "Process object not defined. This object is required to run ldmx-app."
                    );
        }

        PyObject* pProcess = PyObject_GetAttrString(pProcessClass, "lastProcess");
        Py_DECREF(pProcessClass); //don't need the Process class anymore
        if (pProcess == 0) {
            //wasn't able to get lastProcess class member
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", 
                    "Process object not defined. This object is required to run."
                    );
        } else if ( pProcess == Py_None ) {
            //lastProcess was left undefined
            EXCEPTION_RAISE("ConfigureError", 
                    "Process object not defined. This object is required to run."
                    );
        }

        //okay, now we have fully imported the script and gotten the handle
        //to the last Process object defined in the script.
        //We can now look at pProcess and get all of our parameters out of it.

        configuration_.setParameters(getMembers(pProcess));

        //all done with python nonsense
        //delete one parent python object
        //MEMORY still not sure if this is enough, but not super worried about it 
        //  because this only happens once per run
        Py_DECREF(pProcess); 
        //close up python interpreter
        if ( Py_FinalizeEx() < 0 ) {
            PyErr_Print();
            EXCEPTION_RAISE(
                    "PyError",
                    "I wasn't able to close up the python interpreter!"
                    );
        }
    }

    ProcessHandle ConfigurePython::makeProcess() {

        //no python nonsense happens in here,
        //this just takes the parameters determined earlier
        //and puts them into the Process + EventProcessor framework

        auto process{std::make_unique<Process>(
                configuration_.getParameter<std::string>("passName")
                )};  

        process->setHistogramFileName(configuration_.getParameter<std::string>("histogramFile"));
        process->setEventLimit(configuration_.getParameter<int>("maxEvents"));
        process->setLogFrequency(configuration_.getParameter<int>("logFrequency")); 
        process->setCompressionSetting(configuration_.getParameter<int>("compressionSetting"));

        auto run{configuration_.getParameter<int>("run")};
        if ( run > 0 ) process->setRunNumber(run);

        auto libs{configuration_.getParameter<std::vector<std::string>>("libraries")};
        std::for_each(libs.begin(), libs.end(), 
                [](auto& lib) { EventProcessorFactory::getInstance().loadLibrary(lib);}
                ); 

        auto inputFiles{configuration_.getParameter<std::vector<std::string>>("inputFiles")};
        for (auto file : inputFiles ) {
            process->addFileToProcess(file);
        }

        auto outputFiles{configuration_.getParameter<std::vector<std::string>>("outputFiles")};
        for (auto file : outputFiles) {
            process->addOutputFileName(file);
        }

        auto keepRules{configuration_.getParameter<std::vector<std::string>>("keep")};
        for (auto rule : keepRules) {
            process->addDropKeepRule(rule);
        }

        process->getStorageController().setDefaultKeep(
                configuration_.getParameter<bool>("skimDefaultIsKeep")
                );
        auto skimRules{configuration_.getParameter<std::vector<std::string>>("skimRules")};
        for (size_t i=0; i<skimRules.size(); i+=2) {
            process->getStorageController().addRule(skimRules[i],skimRules[i+1]);
        }

        auto sequence{configuration_.getParameter<std::vector<Parameters>>("sequence")};
        for (auto proc : sequence) {
            auto className{proc.getParameter<std::string>("className")};
            auto instanceName{proc.getParameter<std::string>("instanceName")};
            EventProcessor* ep = EventProcessorFactory::getInstance().createEventProcessor(
                    className, instanceName, *process);
            if (ep == 0) {
                EXCEPTION_RAISE("UnableToCreate", 
                        "Unable to create instance '" + instanceName + "' of class '" + className 
                        + "'. Did you load the library that this class is apart of?");
            }
            auto histograms{proc.getParameter<std::vector<Parameters>>("histograms")};
            if (!histograms.empty()) {
                ep->getHistoDirectory();
                ep->createHistograms( histograms );
            }
            ep->configure(proc);
            process->addToSequence(ep);
        }

        return process;
    }

} //ldmx
