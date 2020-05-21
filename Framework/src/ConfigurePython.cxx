/**
 * @file ConfigurePython.cxx
 * @brief Utility class that reads/executes a python script and creates a 
 *        Process object based on the input.
 * @author Jeremy Mans, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Framework/ConfigurePython.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/HistogramPool.h"
#include "Framework/EventProcessorFactory.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>
#include <cstring>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TH1F.h"

namespace ldmx {

    /**
     * Converts a char string to a wide char string.
     *
     * @note
     * Newly allocates the returned object,
     * so make sure to cleanup.
     *
     * @param[in] cstr char string to translate
     * @return newly allocated wide char string
     */
    static wchar_t* getWC(const char *cstr) {
        const size_t cSize = mbstowcs(NULL,cstr,0)+1;
        wchar_t* wc = new wchar_t[cSize];
        mbstowcs( wc , cstr , cSize );
        return wc;
    }

    /**
     * Check if the input python object is a string
     *
     * Simple enough, only a function to isolate the
     * if-else compile-time macro.
     *
     * @param[in] python object
     * @return true if a string python object
     */
    static bool isPyString(PyObject* pyObj) {
#if PY_MAJOR_VERSION < 3
        return PyString_Check(pyObj);
#else
        return PyUnicode_Check(pyObj);
#endif
    }

    /**
     * Check if the input python object is an integer
     *
     * Simple enough, only a function to isolate the
     * if-else compile-time macro.
     *
     * @param[in] python object
     * @return true if an integer python object
     */
    static bool isPyInt(PyObject* pyObj) {
#if PY_MAJOR_VERSION < 3
        return PyInt_Check(pyObj);
#else
        return PyLong_Check(pyObj);
#endif
    }

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
#if PY_MAJOR_VERSION < 3
        retval = PyString_AsString(pyObj);
#else
        PyObject* pyStr = PyUnicode_AsEncodedString(pyObj, "utf-8","Error ~");
        retval = PyBytes_AS_STRING(pyStr);
        Py_XDECREF(pyStr);
#endif
        return retval;
    }

    /**
     * Turn the input python int object into a C++ int.
     *
     * Only a function to isolate the if-else compile-time macro.
     *
     * @param[in] python object assumed to be an int python object
     * @return the value stored in it
     */
    static int getPyInt(PyObject* pyObj) {
        int retval(0);
#if PY_MAJOR_VERSION < 3
        retval = PyInt_AsLong(pyObj);
#else
        retval = PyLong_AsLong(pyObj);
#endif
        return retval;
    }

    /**
     * Get a string object member of the input owner.
     * Otherwise, return the empty string.
     *
     * @param[in] owner python object to look for the string member
     * @param[in] name name of string member of python object
     * @return value of string member
     */
    static std::string stringMember(PyObject* owner, const std::string& name) {
        std::string retval;
        PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
        if (temp != 0) {
            retval = getPyString(temp);
            Py_DECREF(temp);
        }
        return retval;
    }

    /**
     * Get a integer member of the input owner.
     *
     * @param[in] owner python object to look for the integer member
     * @param[in] name name of integer member of python object
     * @return value of integer member
     */
    static long intMember(PyObject* owner, const std::string& name) {
        long retval(0);
        PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
        if (temp != 0) {
            retval = getPyInt(temp);
            Py_DECREF(temp);
        }
        return retval;
    }

    /**
     * Get a string list member of the input owner.
     * 
     * @throw Exception if input name does not reference a python list object.
     *
     * @param[in] owner python object to look for the list member
     * @param[in] name name of list member of python object
     * @return vector of strings containing the entries in the python member list
     */
    static std::vector<std::string> stringListMember(PyObject* owner, const char * listname ) {
        auto pylist = PyObject_GetAttrString( owner , listname );
        if ( !PyList_Check(pylist) ) {
            EXCEPTION_RAISE(
                    "BadType",
                    "'" + std::string(listname) + "' is not a python list as expected."
                    );
        }

        std::vector<std::string> list;
        for ( Py_ssize_t i = 0; i < PyList_Size(pylist); i++ ) {
            PyObject *elem = PyList_GetItem(pylist , i );
            list.push_back( getPyString( elem ) );
            Py_DECREF( elem );
        }
        Py_DECREF(pylist);

        return std::move(list);
    }

    ConfigurePython::ConfigurePython(const std::string& pythonScript, char* args[], int nargs) {

        //assumes that nargs >= 0
        //  this is true always because we error out if no python script has been found

        std::string cmd = pythonScript;
        if (pythonScript.rfind("/") != std::string::npos) {
            cmd = pythonScript.substr(pythonScript.rfind("/") + 1);
        }
        cmd = cmd.substr(0, cmd.find(".py"));

        Py_Initialize();

        //python needs the argument list as if you are on the command line
        //  targs = [ script , arg0 , arg1 , ... ] ==> len(targs) = nargs+1
        
        //The third argument to PySys_SetArgvEx tells python to import
        //the args and add the directory of the first argument to
        //the PYTHONPATH
        //This way, the command to import the module just needs to be
        //the name of the python script
#if PY_MAJOR_VERSION < 3
        char** targs = new char*[nargs+1];
        targs[0] = (char*) pythonScript.c_str();
        for (int i = 0; i < nargs; i++)
            targs[i+1] = args[i];
        PySys_SetArgvEx(nargs+1, targs, 1);
        delete [ ] targs; //1D array because args is owned by main
#else
        //PySys_SetArgvEx uses wchar_t instead of char in python3
        wchar_t** targs = new wchar_t*[nargs+1];
        targs[0] = getWC(pythonScript.c_str());
        for (int i = 0; i < nargs;  i++)
            targs[i+1] = getWC(args[i]);
        PySys_SetArgvEx(nargs+1, targs, 1);
        //clean up the 2D character array
        for ( int i = 0; i < nargs+1; i++ )
            delete [] targs[i];
        delete [] targs;
#endif

        //the following line is what actually runs the script
        PyObject* script = PyImport_ImportModule(cmd.c_str());

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
                    "Process object not defined. This object is required to run ldmx-app."
                    );
        } else if ( pProcess == Py_None ) {
            //lasProcess was left undefined
            EXCEPTION_RAISE("ConfigureError", 
                    "Process object not defined. This object is required to run ldmx-app."
                    );
        }

        //okay, now we have fully imported the script and gotten the handle
        //to the last Process object defined in the script.
        //We can now look at pProcess and get all of our parameters out of it.

        keepRules_    = stringListMember( pProcess , "keep" );
        skimRules_    = stringListMember( pProcess , "skimRules" );
        inputFiles_   = stringListMember( pProcess , "inputFiles" );
        outputFiles_  = stringListMember( pProcess , "outputFiles" );
        libraries_    = stringListMember( pProcess , "libraries" );
        eventLimit_   = intMember( pProcess , "maxEvents" );
        run_          = intMember( pProcess , "run" );
        logFrequency_ = intMember( pProcess , "logFrequency" ); 
        histoOutFile_ = stringMember( pProcess , "histogramFile" );
        passname_     = stringMember( pProcess , "passName" );

        skimDefaultIsKeep_ = bool(intMember(pProcess, "skimDefaultIsKeep"));

        //Now the more complicated paramter: sequence
        //  we need to go through this list and attach all of the parameters
        //  of each processor to itself as well
        PyObject* pysequence = PyObject_GetAttrString( pProcess , "sequence" );
        for (auto i{0}; i < PyList_Size(pysequence); ++i) {

            PyObject* processor{PyList_GetItem(pysequence, i)}; 

            ProcessorClass pc; 
            pc.className_ = stringMember(processor, "className");
            pc.instanceName_ = stringMember(processor, "instanceName");

            PyObject* histos{PyObject_GetAttrString(processor, "histograms")};

            for (auto ihisto{0}; ihisto < PyList_Size(histos); ++ihisto) {

                PyObject* histogram{PyList_GetItem(histos, ihisto)};

                HistogramInfo histInfo; 
                histInfo.name_   = stringMember(histogram, "name"); 
                histInfo.xLabel_ = stringMember(histogram, "xlabel"); 
                histInfo.bins_   = intMember(histogram, "bins"); 
                histInfo.xmin_   = intMember(histogram, "xmin"); 
                histInfo.xmax_   = intMember(histogram, "xmax");  

                pc.histograms_.push_back(histInfo); 

                Py_DECREF(histogram);
            }
            Py_DECREF(histos);

            PyObject* parameters{PyObject_GetAttrString(processor, "parameters")};
            if (parameters != 0 && PyDict_Check(parameters)) {

                auto params{getParameters(parameters)}; 

                pc.params_.setParameters(params); 
            }
            Py_DECREF(parameters);

            sequence_.push_back(pc);

            Py_DECREF(processor);
        }
        Py_DECREF(pysequence);

        //all done with python nonsense
#if PY_MAJOR_VERSION < 3
        //do nothing for some reason
        //  too lazy to figure out how to close up python2 well
#else
        Py_Finalize();
#endif
    }

    Process* ConfigurePython::makeProcess() {

        //no python nonsense happens in here,
        //this just takes the parameters determined earlier
        //and puts them into the Process + EventProcessor framework

        auto process{std::make_unique<Process>(passname_)};  

        process->setHistogramFileName(histoOutFile_);
        process->setEventLimit(eventLimit_);
        process->setLogFrequency(logFrequency_); 

        std::for_each(libraries_.begin(), libraries_.end(), 
                [](auto& lib) { EventProcessorFactory::getInstance().loadLibrary(lib);}
                ); 

        for (auto proc : sequence_) {
            EventProcessor* ep = EventProcessorFactory::getInstance().createEventProcessor(proc.className_, proc.instanceName_, *process);
            if (ep == 0) {
                EXCEPTION_RAISE("UnableToCreate", "Unable to create instance '" + proc.instanceName_ + "' of class '" + proc.className_ + "'");
            }

            if (!proc.histograms_.empty()) {
                HistogramPool* histograms = HistogramPool::getInstance(); 
                ep->getHistoDirectory();
                for (const auto& hist : proc.histograms_) { 
                    histograms->create<TH1F>(hist.name_, hist.xLabel_, hist.bins_, hist.xmin_, hist.xmax_); 
                } 
            }
            ep->configure(proc.params_);
            process->addToSequence(ep);
        }
        for (auto file : inputFiles_) {
            process->addFileToProcess(file);
        }
        for (auto file : outputFiles_) {
            process->addOutputFileName(file);
        }
        for (auto rule : keepRules_) {
            process->addDropKeepRule(rule);
        }
        process->getStorageController().setDefaultKeep(skimDefaultIsKeep_);
        for (size_t i=0; i<skimRules_.size(); i+=2) {
            process->getStorageController().addRule(skimRules_[i],skimRules_[i+1]);
        }
        if (run_ > 0)
            process->setRunNumber(run_);

        return process.release();
    }

    std::map< std::string, std::any > ConfigurePython::getParameters(PyObject* dictionary) { 

        PyObject *key(0), *value(0);
        Py_ssize_t pos = 0;

        std::map < std::string, std::any > params; 

        while (PyDict_Next(dictionary, &pos, &key, &value)) {

            std::string skey{getPyString(key)};

            if (isPyInt(value)) {
                if (PyBool_Check(value)) {
                    params[skey] = bool(PyLong_AsLong(value)); 
                } else { 
                    params[skey] = int(PyLong_AsLong(value));
                }
            } else if (PyFloat_Check(value)) {
                params[skey] = PyFloat_AsDouble(value);  
            } else if (isPyString(value)) {
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

                    } else if (isPyString(vec0)) {
                        std::vector<std::string> vals;
                        for (Py_ssize_t j = 0; j < PyList_Size(value); j++){
                            PyObject* elem = PyList_GetItem(value , j );
                            vals.push_back(getPyString(elem));
                            Py_DECREF(elem);
                        }

                        params[skey] = vals;

                    } else { 

                        // If the objects stored in the list doesn't 
                        // satisfy any of the above conditions, just
                        // create a vector of Class objects.
                        std::vector< Class > vals;
                        for (auto j{0}; j < PyList_Size(value); ++j) {

                            auto object{PyList_GetItem( value, j )}; 
                            Class c; 
                            c.className_    = stringMember(object, "class_name");
                            c.instanceName_ = stringMember(object, "instance_name");  

                            auto dict{PyObject_GetAttrString(object, "parameters")};
                            auto classParams{getParameters(dict)}; 
                            c.params_.setParameters(classParams); 
                            Py_DECREF(dict);

                            vals.push_back(c);

                            Py_DECREF(object);
                        } 
                        params[skey] = vals;

                    } //type of object in python list
                } //python list has non-zero size
            } //python object type

            Py_XDECREF( key );
            Py_XDECREF(value);

        } //loop through python dictionary

        return params; 
    }
} //ldmx
