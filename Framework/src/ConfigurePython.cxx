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

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TH1F.h"

namespace ldmx {

    static bool isPyString(PyObject* pyObj) {
#if PY_MAJOR_VERSION < 3
        return PyString_Check(pyObj);
#else
        return PyUnicode_Check(pyObj);
#endif
    }

    static bool isPyInt(PyObject* pyObj) {
#if PY_MAJOR_VERSION < 3
        return PyInt_Check(pyObj);
#else
        return PyLong_Check(pyObj);
#endif
    }

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

    static int getPyInt(PyObject* pyObj) {
        int retval(0);
#if PY_MAJOR_VERSION < 3
        retval = PyInt_AsLong(pyObj);
#else
        retval = PyLong_AsLong(pyObj);
#endif
        return retval;
    }

    static std::string stringMember(PyObject* owner, const std::string& name) {
        std::string retval;
        PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
        if (temp != 0) {
            retval = getPyString(temp);
            Py_DECREF(temp);
        }
        return retval;
    }

    static long intMember(PyObject* owner, const std::string& name) {
        long retval(0);
        PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
        if (temp != 0) {
            retval = getPyInt(temp);
            Py_DECREF(temp);
        }
        return retval;
    }

    static std::vector<std::string> stringListMember(PyObject* processHandle, const char * listname ) {
        auto pylist = PyObject_GetAttrString( processHandle , listname );
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
        std::string path(".");
        std::string cmd = pythonScript;

        // if a path was specified, get that out
        if (pythonScript.rfind("/") != std::string::npos) {
            path = pythonScript.substr(0, pythonScript.rfind("/"));
            cmd = pythonScript.substr(pythonScript.rfind("/") + 1);
        }
        cmd = cmd.substr(0, cmd.find(".py"));

        Py_Initialize();
        if (nargs > 0) {
#if PY_MAJOR_VERSION < 3
            char** targs = new char*[nargs + 1];
            targs[0] = (char*) pythonScript.c_str();
            for (int i = 0; i < nargs; i++)
                targs[i + 1] = args[i];
            PySys_SetArgvEx(nargs, targs, 1);
#else
            wchar_t** targs = new wchar_t*[nargs];
            std::cout << "nargs: " << nargs << std::endl;
            std::cout << "Load Python Script Name " << pythonScript.c_str() << std::endl;
            PyObject *tmpstr = PyUnicode_FromString(pythonScript.c_str());
            targs[0] = PyUnicode_AsWideCharString(tmpstr,NULL);
            Py_DECREF(tmpstr);
            for (int i = 0; i < nargs - 1; i++){
                std::cout << "Load arg " << i << "   " << args[i] << std::endl;
                tmpstr = PyUnicode_FromString(args[i]);
                targs[i+1] = PyUnicode_AsWideCharString(tmpstr,NULL);
                Py_DECREF(tmpstr);
            }
            std::cout << "Set Python Args" << std::endl;
            PySys_SetArgvEx(nargs-1, targs, 1);
            std::cout << "Free Memory" << std::endl;
#endif
            delete[] targs;
            std::cout << "Free Memory Succeed" << std::endl;
        }

        PyObject *script, *temp;
        std::cout << "Load cmd string " << cmd.c_str() << std::endl;
        //what does temp do for us???
#if PY_MAJOR_VERSION < 3
        temp = PyString_FromString(cmd.c_str());
#else
        temp = PyUnicode_FromString(cmd.c_str());
#endif
        //the following line is what actually runs the script
        script = PyImport_ImportModule(cmd.c_str());
        Py_DECREF(temp);

        if (script == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
        }

        PyObject* pCMod = PyObject_GetAttrString(script, "ldmxcfg");
        //TODO check if Py_DECREF(script) works here
        if (pCMod == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
        }

        PyObject* pProcessClass = PyObject_GetAttrString(pCMod, "Process");
        Py_DECREF(pCMod);
        if (pProcessClass == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", 
                    "Process object not defined. This object is required to run ldmx-app."
                    );
        }

        PyObject* pProcess = PyObject_GetAttrString(pProcessClass, "lastProcess");
        Py_DECREF(pProcessClass);
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

        passname_ = stringMember(pProcess, "passName");
        eventLimit_ = intMember(pProcess, "maxEvents");
        run_ = intMember(pProcess, "run");
        histoOutFile_ = stringMember(pProcess, "histogramFile");

        // Get the print frequency
        logFrequency_ = intMember(pProcess, "logFrequency"); 

        PyObject* pysequence = PyObject_GetAttrString( pProcess , "sequence" );
        for (auto i{0}; i < PyList_Size(pysequence); ++i) {

            auto processor{PyList_GetItem(pysequence, i)}; 

            ProcessorClass pc; 
            pc.className_ = stringMember(processor, "className");
            pc.instanceName_ = stringMember(processor, "instanceName");

            auto histos{PyObject_GetAttrString(processor, "histograms")};

            for (auto ihisto{0}; ihisto < PyList_Size(histos); ++ihisto) {

                auto histogram{PyList_GetItem(histos, ihisto)};

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

            auto parameters{PyObject_GetAttrString(processor, "parameters")};
            if (parameters != 0 && PyDict_Check(parameters)) {

                auto params{getParameters(parameters)}; 

                pc.params_.setParameters(params); 
            }
            Py_DECREF(parameters);

            sequence_.push_back(pc);

            Py_DECREF(processor);
        }
        Py_DECREF(pysequence);


        skimDefaultIsKeep_ = bool(intMember(pProcess, "skimDefaultIsKeep"));

        keepRules_   = stringListMember( pProcess , "keep" );
        skimRules_   = stringListMember( pProcess , "skimRules" );
        inputFiles_  = stringListMember( pProcess , "inputFiles" );
        outputFiles_ = stringListMember( pProcess , "outputFiles" );
        libraries_   = stringListMember( pProcess , "libraries" );

        //not really sure where this should go
        Py_DECREF(script);
    }

    ConfigurePython::~ConfigurePython() {
        Py_Finalize();
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
