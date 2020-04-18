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

    static std::string stringMember(PyObject* owner, const std::string& name) {
        std::string retval;
        PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
        if (temp != 0) {
            retval = PyString_AsString(temp);
            Py_DECREF(temp);
        }
        return retval;
    }

    static long intMember(PyObject* owner, const std::string& name) {
        long retval(0);
        PyObject* temp = PyObject_GetAttrString(owner, name.c_str());
        if (temp != 0) {
            retval = PyInt_AsLong(temp);
            Py_DECREF(temp);
        }
        return retval;
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
            char** targs = new char*[nargs + 1];
            targs[0] = (char*) pythonScript.c_str();
            for (int i = 0; i < nargs; i++)
                targs[i + 1] = args[i];
            PySys_SetArgvEx(nargs, targs, 1);
            delete[] targs;
        }

        PyObject* script, *temp, *pylist;
        temp = PyString_FromString(cmd.c_str());
        script = PyImport_ImportModule(cmd.c_str());
        Py_DECREF(temp);

        if (script == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
        }

        PyObject* pCMod = PyObject_GetAttrString(script, "ldmxcfg");
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

        PyObject* pysequence = PyObject_GetAttrString(pProcess, "sequence");
        if (!PyList_Check(pysequence)) {
            EXCEPTION_RAISE("ConfigureError", "sequence is not a python list as expected.");
        }

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
            }
            Py_DECREF(histos);

            auto parameters{PyObject_GetAttrString(processor, "parameters")};
            if (parameters != 0 && PyDict_Check(parameters)) {

                PyObject *key(0), *value(0);
                Py_ssize_t pos = 0;

                auto params{getParameters(parameters)}; 
                
                pc.params_.setParameters(params); 
            }

            sequence_.push_back(pc);
        }
        Py_DECREF(pysequence);

        pylist = PyObject_GetAttrString(pProcess, "keep");
        if (!PyList_Check(pylist)) {
            std::cerr << "keep is not a python list as expected.\n";
            return;
        }
        for (Py_ssize_t i = 0; i < PyList_Size(pylist); i++) {
            PyObject* elem = PyList_GetItem(pylist, i);
            keepRules_.push_back(PyString_AsString(elem));
        }
        Py_DECREF(pylist);

        skimDefaultIsKeep_=intMember(pProcess, "skimDefaultIsKeep");
        pylist = PyObject_GetAttrString(pProcess, "skimRules");
        if (!PyList_Check(pylist)) {
            std::cerr << "skimRules is not a python list as expected.\n";
            return;
        }
        for (Py_ssize_t i = 0; i < PyList_Size(pylist); i++) {
            PyObject* elem = PyList_GetItem(pylist, i);
            skimRules_.push_back(PyString_AsString(elem));
        }
        Py_DECREF(pylist);

        pylist = PyObject_GetAttrString(pProcess, "inputFiles");
        if (!PyList_Check(pylist)) {
            std::cerr << "inputFiles is not a python list as expected.\n";
            return;
        }
        for (Py_ssize_t i = 0; i < PyList_Size(pylist); i++) {
            PyObject* elem = PyList_GetItem(pylist, i);
            inputFiles_.push_back(PyString_AsString(elem));
        }
        Py_DECREF(pylist);

        pylist = PyObject_GetAttrString(pProcess, "outputFiles");
        if (!PyList_Check(pylist)) {
            std::cerr << "outputFiles is not a python list as expected.\n";
            return;
        }
        for (Py_ssize_t i = 0; i < PyList_Size(pylist); i++) {
            PyObject* elem = PyList_GetItem(pylist, i);
            outputFiles_.push_back(PyString_AsString(elem));
        }
        Py_DECREF(pylist);

        pylist = PyObject_GetAttrString(pProcess, "libraries");
        if (!PyList_Check(pylist)) {
            std::cerr << "libraries is not a python list as expected.\n";
            return;
        }
        for (Py_ssize_t i = 0; i < PyList_Size(pylist); i++) {
            PyObject* elem = PyList_GetItem(pylist, i);
            libraries_.push_back(PyString_AsString(elem));
        }
        Py_DECREF(pylist);
    }

    ConfigurePython::~ConfigurePython() {
        //MEMORY 'Use of uninitialised value of size 8'
        //  Trickles down from here to PyObject_Free
        //  Missing a PyObject_Free earlier? Move this to end of constructor?
        Py_Finalize();
    }

    ProcessHandle ConfigurePython::makeProcess() {

        ProcessHandle process{std::make_unique<Process>(passname_)};  

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

        return process;
    }

    std::map< std::string, std::any > ConfigurePython::getParameters(PyObject* dictionary) { 

        PyObject *key(0), *value(0);
        Py_ssize_t pos = 0;

        std::map < std::string, std::any > params; 

        while (PyDict_Next(dictionary, &pos, &key, &value)) {

            std::string skey{PyString_AsString(key)};
    
            if (PyInt_Check(value)) {
                if (PyBool_Check(value)) {
                    params[skey] = bool(PyInt_AsLong(value)); 
                } else { 
                    params[skey] = int(PyInt_AsLong(value));
                }
            } else if (PyFloat_Check(value)) {
                params[skey] = PyFloat_AsDouble(value);  
            } else if (PyString_Check(value)) {
                params[skey] = std::string(PyString_AsString(value));
            } else if (PyList_Check(value)) { // assume everything is same value as first value
                if (PyList_Size(value) > 0) {

                    auto vec0{PyList_GetItem(value, 0)};

                    if (PyInt_Check(vec0)) {
                        std::vector<int> vals;

                        for (auto j{0}; j < PyList_Size(value); j++)
                            vals.push_back(PyInt_AsLong(PyList_GetItem(value, j)));

                        params[skey] = vals;

                    } else if (PyFloat_Check(vec0)) {
                        std::vector<double> vals;

                        for (auto j{0}; j < PyList_Size(value); j++)
                            vals.push_back(PyFloat_AsDouble(PyList_GetItem(value, j)));

                        params[skey] = vals;

                    } else if (PyString_Check(vec0)) {
                        std::vector<std::string> vals;

                        for (auto j{0}; j < PyList_Size(value); j++)
                            vals.push_back(PyString_AsString(PyList_GetItem(value, j)));

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

                            vals.push_back(c);
                        } 
                        params[skey] = vals;
                    }
                }
            }
        }

        return params; 
    }
}

