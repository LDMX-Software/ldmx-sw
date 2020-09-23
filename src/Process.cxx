/**
 * @file Process.cxx
 * Implementation file for Process class
 */

#include <iostream>
#include "TFile.h"
#include "TROOT.h"
#include "Framework/EventProcessor.h"
#include "Framework/PluginFactory.h"
#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "Framework/Process.h"
#include "Framework/NtupleManager.h"
#include "Framework/Logger.h"
#include "Event/RunHeader.h"
#include "Event/EventHeader.h"
#include "Framework/Exception.h"

namespace ldmx {

  Process::Process(const Parameters& configuration) : conditions_{*this} {

        passname_      = configuration.getParameter<std::string>("passName", "");
        histoFilename_ = configuration.getParameter<std::string>("histogramFile",""); 
        logFileName_   = configuration.getParameter<std::string>("logFileName","");

        maxTries_           = configuration.getParameter<int>("maxTriesPerEvent",1);
        eventLimit_         = configuration.getParameter<int>("maxEvents",-1);
        logFrequency_       = configuration.getParameter<int>("logFrequency",-1); 
        compressionSetting_ = configuration.getParameter<int>("compressionSetting",9);
        termLevelInt_       = configuration.getParameter<int>("termLogLevel",2); 
        fileLevelInt_       = configuration.getParameter<int>("fileLogLevel",0); 

        inputFiles_    = configuration.getParameter<std::vector<std::string>>("inputFiles" ,{});
        outputFiles_   = configuration.getParameter<std::vector<std::string>>("outputFiles",{});
        dropKeepRules_ = configuration.getParameter<std::vector<std::string>>("keep"       ,{});

        eventHeader_   = 0;

        auto run{configuration.getParameter<int>("run",-1)};
        if ( run > 0 ) runForGeneration_ = run;

        auto libs{configuration.getParameter<std::vector<std::string>>("libraries",{})};
        std::for_each(libs.begin(), libs.end(), 
                [](auto& lib) { PluginFactory::getInstance().loadLibrary(lib);}
                ); 

        m_storageController.setDefaultKeep(
                       configuration.getParameter<bool>("skimDefaultIsKeep",true)
                );
        auto skimRules{configuration.getParameter<std::vector<std::string>>("skimRules",{})};
        for (size_t i=0; i<skimRules.size(); i+=2) {
            m_storageController.addRule(skimRules[i],skimRules[i+1]);
        }

        auto sequence{configuration.getParameter<std::vector<Parameters>>("sequence",{})};
        if ( sequence.empty() && configuration.getParameter<bool>("testingMode",false)) {     
            EXCEPTION_RAISE(
                    "NoSeq",
                    "No sequence has been defined. What should I be doing?\nUse p.sequence to tell me what processors to run."
                    );
        }
        for (auto proc : sequence) {
            auto className{proc.getParameter<std::string>("className")};
            auto instanceName{proc.getParameter<std::string>("instanceName")};
            EventProcessor* ep = PluginFactory::getInstance().createEventProcessor(
                    className, instanceName, *this);
            if (ep == 0) {
                EXCEPTION_RAISE("UnableToCreate", 
                        "Unable to create instance '" + instanceName + "' of class '" + className 
                        + "'. Did you load the library that this class is apart of?");
            }
            auto histograms{proc.getParameter<std::vector<Parameters>>("histograms",{})};
            if (!histograms.empty()) {
                ep->getHistoDirectory();
                ep->createHistograms( histograms );
            }
            ep->configure(proc);
            sequence_.push_back(ep);
        }

        auto conditionsObjectProviders{configuration.getParameter<std::vector<Parameters> >("conditionsObjectProviders",{})};
        for (auto cop : conditionsObjectProviders) {
      
            auto className{cop.getParameter<std::string>("className")};
            auto objectName{cop.getParameter<std::string>("objectName")};
            auto tagName{cop.getParameter<std::string>("tagName")};

            conditions_.createConditionsObjectProvider(className, objectName, tagName, cop);
        }
    }

    Process::~Process() {
        for ( EventProcessor *ep : sequence_ ) {
            delete ep;
        }
    }

    void Process::run() {

        // set up the logging for this run
        logging::open(
                logging::convertLevel(termLevelInt_),
                logging::convertLevel(fileLevelInt_),
                logFileName_ //if this is empty string, no file is logged to
                );

        // create a logger for this process
        //      other objects will have their own channels
        //      the ldmx_log macro uses a variable called theLog_,
        //      so we are going to name it that for now.
        auto theLog_{logging::makeLogger("Process")};

        // Counter to keep track of the number of events that have been 
        // procesed
        auto n_events_processed{0};

        //event bus for this process
        Event theEvent(passname_);
        
        // Start by notifying everyone that modules processing is beginning
        conditions_.onProcessStart();
        for (auto module : sequence_) module->onProcessStart();

        // If we have no input files, but do have an event number, run for
        // that number of events and generate an output file.
        if (inputFiles_.empty() && eventLimit_ > 0) {
            
            if ( outputFiles_.empty() ) {
                EXCEPTION_RAISE(
                        "InvalidConfig",
                        "No input files or output files were given."
                        );
            } else if ( outputFiles_.size() > 1 ) {
                ldmx_log(warn) << "Several output files given with no input files. "
                    << "Only the first output file '" << outputFiles_.at(0) << "' will be used.";
            }
            std::string outputFileName = outputFiles_.at(0);
            
            EventFile outFile(outputFileName, compressionSetting_);

            for (auto module : sequence_) module->onFileOpen(outFile);

            outFile.setupEvent(&theEvent);
            
            for ( auto rule : dropKeepRules_ ) outFile.addDrop(rule);

            RunHeader runHeader(runForGeneration_);
            runHeader.setRunStart(std::time(nullptr)); //set run starting
            runHeader_ = &runHeader; //give handle to run header to process
            outFile.writeRunHeader(runHeader); //add run header to file

            for (auto module : sequence_)
                if (dynamic_cast<Producer*>(module))
                    dynamic_cast<Producer*>(module)->beforeNewRun(runHeader);

            //now run header has been modified by Producers, so it is valid to read from
            conditions_.onNewRun(runHeader);
            for (auto module : sequence_)
                module->onNewRun(runHeader);

            int numTries = 0; //number of tries for the current event number
            while (n_events_processed < eventLimit_) {
                EventHeader& eh = theEvent.getEventHeader();
                eh.setRun(runForGeneration_);
                eh.setEventNumber(n_events_processed + 1);
                eh.setTimestamp(TTimeStamp());

                // event header pointer grab
                eventHeader_=theEvent.getEventHeaderPtr();
            
                numTries++;

                // reset the storage controller state
                m_storageController.resetEventState();

                if ( numTries <= 1 and getLogFrequency() > 0 and (eh.getEventNumber() % getLogFrequency() == 0 ) ) {
                    TTimeStamp t;
                    ldmx_log(info) << "Processing " << n_events_processed + 1 
                        << " Run " << theEvent.getEventHeader().getRun() 
                        << " Event " << theEvent.getEventHeader().getEventNumber() 
                        << "  (" << t.AsString("lc") << ")";
                }

                bool eventAborted = false;
                for (auto module : sequence_) {
                    try {
                        if (dynamic_cast<Producer*>(module)) {
                            (dynamic_cast<Producer*>(module))->produce(theEvent);
                        } else if (dynamic_cast<Analyzer*>(module)) {
                            (dynamic_cast<Analyzer*>(module))->analyze(theEvent);
                        }
                    } catch( AbortEventException& ) {
                        eventAborted = true;
                        break;
                    }
                }
                
                outFile.nextEvent( eventAborted ? false : m_storageController.keepEvent() /*ignore storage control if event aborted*/);

                if ( not eventAborted or numTries >= maxTries_ ) {
                    n_events_processed++; //increment events made
                    NtupleManager::getInstance().fill();  //fill ntuples
                    numTries = 0; //reset try counter
                }

                NtupleManager::getInstance().clear(); 
                theEvent.Clear();
            }

            for (auto module : sequence_) module->onFileClose(outFile);
            
            runHeader.setRunEnd(std::time(nullptr));
            ldmx_log(info) << runHeader;
            outFile.close();
            
        } else {
            //there are input files

            EventFile* outFile(0);

            bool singleOutput = false;
            if ( outputFiles_.size() == 1 ) {
                singleOutput = true;
            } else if ( !outputFiles_.empty() and outputFiles_.size() != inputFiles_.size() ) {
                EXCEPTION_RAISE( 
                        "Process" ,
                        "Unable to handle case of different number of input and output files (other than zero/one ouput file)."
                        );
            }

            // next, loop through the files
            int ifile = 0;
            int wasRun = -1;
            for (auto infilename : inputFiles_) {

                EventFile inFile(infilename);

                ldmx_log(info) << "Opening file " << infilename;

                for (auto module : sequence_) module->onFileOpen(inFile);
                
                //configure event file that will be iterated over
                EventFile* masterFile; 
                if ( !outputFiles_.empty() ) {

                    //setup new output file if either
                    // 1) we are not in single output mode
                    // 2) this is the first input file
                    if ( !singleOutput or ifile == 0 ) {
                        //setup new output file
                        outFile = new EventFile(outputFiles_[ifile], &inFile, singleOutput, compressionSetting_ );
                        ifile++;

                        //setup theEvent we will iterate over
                        if (outFile) {
                            outFile->setupEvent( &theEvent );
                            masterFile = outFile;
                        } else {
                            EXCEPTION_RAISE(
                                    "Process",
                                    "Unable to construct output file for " + outputFiles_[ifile]
                                    );
                        }

                        for ( auto rule : dropKeepRules_ ) outFile->addDrop(rule);

                    } else {
                        //all other input files
                        outFile->updateParent( &inFile );
                        masterFile = outFile;

                    } //check if in singleOutput mode

                } else {
                    //empty output file list, use inputFile as master file
                    inFile.setupEvent( &theEvent );
                    masterFile = &inFile;
                }

                bool eventAborted = false;
                while (masterFile->nextEvent(eventAborted ? false : m_storageController.keepEvent()/*ignore storage controller if event aborted*/) 
                        && (eventLimit_ < 0 || (n_events_processed) < eventLimit_)) {
                    // clean up for storage control calculation
                    m_storageController.resetEventState();

                    // event header pointer grab
                    eventHeader_=theEvent.getEventHeaderPtr();
        
                    // notify for new run if necessary
                    if (theEvent.getEventHeader().getRun() != wasRun) {
                        wasRun = theEvent.getEventHeader().getRun();
                        try {
                            auto runHeader = masterFile->getRunHeader(wasRun);
                            runHeader_ = &runHeader; //save current run header for later
                            ldmx_log(info) << "Got new run header from '" << masterFile->getFileName() << "' ...\n" << runHeader;
                            for (auto module : sequence_)
                                if (dynamic_cast<Producer*>(module))
                                    dynamic_cast<Producer*>(module)->beforeNewRun(runHeader);
                            //now run header has been modified by Producers, so it is valid to read from
                            conditions_.onNewRun(runHeader);
                            for (auto module : sequence_)
                                module->onNewRun(runHeader);
                        } catch (const Exception&) {
                            ldmx_log(warn) << "Run header for run " << wasRun << " was not found!";
                        }
                    }

                    if ( (logFrequency_ != -1) && ((n_events_processed + 1)%logFrequency_ == 0)) { 
                        TTimeStamp t;
                        ldmx_log(info) << "Processing " << n_events_processed + 1 
                                  << " Run " << theEvent.getEventHeader().getRun() 
                                  << " Event " << theEvent.getEventHeader().getEventNumber() 
                                  << "  (" << t.AsString("lc") << ")";
                    }

                    eventAborted = false;
                    for (auto module : sequence_) {
                        try {
                            if (dynamic_cast<Producer*>(module)) {
                                (dynamic_cast<Producer*>(module))->produce(theEvent);
                            } else if (dynamic_cast<Analyzer*>(module)) {
                                (dynamic_cast<Analyzer*>(module))->analyze(theEvent);
                            }
                        } catch( AbortEventException& ) {
                            eventAborted = true;
                            break;
                        }
                    }

                    if ( not eventAborted ) NtupleManager::getInstance().fill(); 
                    NtupleManager::getInstance().clear(); 

                    n_events_processed++;
                } //loop through events

                if (eventLimit_ > 0 && n_events_processed == eventLimit_) {
                    ldmx_log(info) << "Reached event limit of " << eventLimit_ << " events";
                }

                if (eventLimit_ == 0 && n_events_processed > eventLimit_) {
                    ldmx_log(warn) << "Processing interrupted";
                }


                ldmx_log(info) << "Closing file " << infilename;

                for (auto module : sequence_) module->onFileClose(inFile);

                inFile.close();

                // Reset the event in case of multiple input files
                theEvent.onEndOfFile();

                if ( outFile and !singleOutput ) {
                    outFile->close();
                    delete outFile;
                    outFile = nullptr;
                }

            } //loop through input files

            if ( outFile ) {
                //close outFile
                //  outFile would survive to here in single output mode
                outFile->close();
                delete outFile;
                outFile = nullptr;
            }

        } //are there input files? if-else tree

        //close up histogram file if anything was put into it
        if (histoTFile_) {
            histoTFile_->Write();
            delete histoTFile_;
            histoTFile_ = 0;
        }

        // finally, notify everyone that we are stopping
        for (auto module : sequence_) {
            module->onProcessEnd();
        }

        //we're done so let's close up the logging
        logging::close();
    }

    int Process::getRunNumber() const {
        return (eventHeader_)?(eventHeader_->getRun()):(runForGeneration_);
    }
  
    TDirectory* Process::makeHistoDirectory(const std::string& dirName) {
        auto owner{openHistoFile()}; 
        TDirectory* child = owner->mkdir((char*) dirName.c_str());
        if (child)
            child->cd();
        return child;
    }

    TDirectory* Process::openHistoFile() {
        TDirectory* owner{nullptr}; 
        
        if (histoFilename_.empty()) {
            //trying to write histograms/ntuples but no file defined
            EXCEPTION_RAISE(
                    "NoHistFileName",
                    "You did not provide the necessary histogram file name to put your histograms (or ntuples) in.\n    Provide this name in the python configuration with 'p.histogramFile = \"myHistFile.root\"' where p is the Process object."
                    );
        } else if (histoTFile_ == nullptr) { 
            histoTFile_ = new TFile(histoFilename_.c_str(), "RECREATE");
            owner = histoTFile_;
        } else owner = histoTFile_; 
        owner->cd(); 

        return owner; 
    }
}
