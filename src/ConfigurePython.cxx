// python
#include "Python.h"

// LDMX
#include "Framework/ConfigurePython.h"
#include "Framework/HistogramPool.h"
#include "Framework/Process.h"
#include "Framework/EventProcessorFactory.h"

// STL
#include <iostream>

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
        long retval;
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

        PyObject* script, *temp, *process, *pMain, *pylist;
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
            EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
        }

        PyObject* pProcess = PyObject_GetAttrString(pProcessClass, "lastProcess");
        Py_DECREF(pProcessClass);
        if (pProcess == 0) {
            PyErr_Print();
            EXCEPTION_RAISE("ConfigureError", "Problem loading python script");
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
        for (Py_ssize_t i = 0; i < PyList_Size(pysequence); i++) {

            PyObject* processor = PyList_GetItem(pysequence, i);
            ProcessorInfo pi;
            pi.classname_ = stringMember(processor, "className");
            pi.instancename_ = stringMember(processor, "instanceName");

            PyObject* histos = PyObject_GetAttrString(processor, "histograms");
            
            for (Py_ssize_t ihisto{0}; ihisto < PyList_Size(histos); ++ihisto) {
                PyObject* histogram{PyList_GetItem(histos, ihisto)};

                HistogramInfo histInfo; 
                histInfo.name_   = stringMember(histogram, "name"); 
                histInfo.xLabel_ = stringMember(histogram, "xlabel"); 
                histInfo.bins_   = intMember(histogram, "bins"); 
                histInfo.xmin_   = intMember(histogram, "xmin"); 
                histInfo.xmax_   = intMember(histogram, "xmax");  

                pi.histograms_.push_back(histInfo); 
            }
            Py_DECREF(histos);

            PyObject* params = PyObject_GetAttrString(processor, "parameters");
            if (params != 0 && PyDict_Check(params)) {
                PyObject *key(0), *value(0);
                Py_ssize_t pos = 0;

                while (PyDict_Next(params, &pos, &key, &value)) {
                    std::string skey = PyString_AsString(key);
                    if (PyInt_Check(value)) {
                        pi.params_.insert(skey, int(PyInt_AsLong(value)));
                        //printf("Int Key: %s\n",skey.c_str());
                    } else if (PyFloat_Check(value)) {
                        pi.params_.insert(skey, PyFloat_AsDouble(value));
                        //printf("Double Key: %s\n",skey.c_str());
                    } else if (PyString_Check(value)) {
                        pi.params_.insert(skey, PyString_AsString(value));
                        //printf("String Key: %s\n",skey.c_str());
                    } else if (PyList_Check(value)) { // assume everything is same value as first value
                        if (PyList_Size(value) > 0) {
                            PyObject* vec0 = PyList_GetItem(value, 0);
                            if (PyInt_Check(vec0)) {
                                std::vector<int> vals;
                                for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                                    vals.push_back(PyInt_AsLong(PyList_GetItem(value, j)));
                                pi.params_.insert(skey, vals);
                                //printf("VInt Key: %s\n",skey.c_str());
                            } else if (PyFloat_Check(vec0)) {
                                std::vector<double> vals;
                                for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                                    vals.push_back(PyFloat_AsDouble(PyList_GetItem(value, j)));
                                pi.params_.insert(skey, vals);
                                //printf("VDouble Key: %s\n",skey.c_str());
                            } else if (PyString_Check(vec0)) {
                                std::vector<std::string> vals;
                                for (Py_ssize_t j = 0; j < PyList_Size(value); j++)
                                    vals.push_back(PyString_AsString(PyList_GetItem(value, j)));
                                pi.params_.insert(skey, vals);
                                //printf("VString Key: %s\n",skey.c_str());
                            }
                        }
                    }
                }
            }

            sequence_.push_back(pi);
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
        Py_Finalize();
    }

    Process* ConfigurePython::makeProcess() {
        Process* p = new Process(passname_);

        p->setHistogramFileName(histoOutFile_);
        p->setEventLimit(eventLimit_);
        p->setLogFrequency(logFrequency_); 

        for (auto lib : libraries_) {
            EventProcessorFactory::getInstance().loadLibrary(lib);
        }

        for (auto proc : sequence_) {
            EventProcessor* ep = EventProcessorFactory::getInstance().createEventProcessor(proc.classname_, proc.instancename_, *p);
            if (ep == 0) {
                EXCEPTION_RAISE("UnableToCreate", "Unable to create instance '" + proc.instancename_ + "' of class '" + proc.classname_ + "'");
            }
            
            if (!proc.histograms_.empty()) {
                HistogramPool* histograms = HistogramPool::getInstance(); 
                ep->getHistoDirectory();
                for (const auto& hist : proc.histograms_) { 
                    histograms->create<TH1F>(hist.name_, hist.xLabel_, hist.bins_, hist.xmin_, hist.xmax_); 
                } 
            }
            ep->configure(proc.params_);
            p->addToSequence(ep);
        }
        for (auto file : inputFiles_) {
            p->addFileToProcess(file);
        }
        for (auto file : outputFiles_) {
            p->addOutputFileName(file);
        }
        for (auto rule : keepRules_) {
            p->addDropKeepRule(rule);
        }
        p->getStorageController().setDefaultKeep(skimDefaultIsKeep_);
        for (size_t i=0; i<skimRules_.size(); i+=2) {
            p->getStorageController().addRule(skimRules_[i],skimRules_[i+1]);
        }
        if (run_ > 0)
            p->setRunNumber(run_);

        return p;
    }
}

