#include <iostream>
#include "TFile.h"
#include "TROOT.h"
#include "Framework/EventProcessor.h"
#include "Framework/EventImpl.h"
#include "Framework/EventFile.h"
#include "Framework/Process.h"
#include "Event/RunHeader.h"

namespace ldmx {

    Process::Process(const std::string& passname) :
            passname_ {passname} {
    }

    void Process::run() {

        try {
            int n_events_processed = 0;

            // first, notify everyone that we are starting
            for (auto module : sequence_) {
                module->onProcessStart();
            }

            // if we have no input files, but do have an event number, run for that number of events on an output file
            if (inputFiles_.empty() && eventLimit_ > 0) {
                EventFile outFile(outputFiles_[0], true);

                for (auto module : sequence_) {
                    module->onFileOpen(outputFiles_[0]);
                }

                EventImpl theEvent(passname_);
                outFile.setupEvent(&theEvent);

                while (n_events_processed < eventLimit_) {
                    EventHeader& eh = theEvent.getEventHeaderMutable();
                    eh.setRun(runForGeneration_);
                    eh.setEventNumber(n_events_processed + 1);
                    eh.setTimestamp(TTimeStamp());

                    theEvent.getEventHeader()->Print();

                    // reset the storage controller state
                    m_storageController.resetEventState();

                    for (auto module : sequence_) {
                        if (dynamic_cast<Producer*>(module)) {
                            (dynamic_cast<Producer*>(module))->produce(theEvent);
                        } else if (dynamic_cast<Analyzer*>(module)) {
                            (dynamic_cast<Analyzer*>(module))->analyze(theEvent);
                        }
                    }
                    outFile.nextEvent(m_storageController.keepEvent());
                    theEvent.Clear();
                    n_events_processed++;
                }

                for (auto module : sequence_) {
                    module->onFileClose(outputFiles_[0]);
                }
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

                    EventImpl theEvent(passname_);

                    std::cout << "[ Process ] : Opening file " << infilename << std::endl;

                    for (auto module : sequence_) {
                        module->onFileOpen(infilename);
                    }
                    
                    //configure event file that will be iterated over
                    EventFile* masterFile; 
                    if ( !outputFiles_.empty() ) {

                        //setup new output file if either
                        // 1) we are not in single output mode
                        // 2) this is the first input file
                        if ( !singleOutput or ifile == 0 ) {
                            //setup new output file
                            outFile = new EventFile(outputFiles_[ifile], &inFile, singleOutput );
                            ifile++;

                            for ( auto rule : dropKeepRules_ ) {
                                outFile->addDrop(rule);
                            }

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

                    while (masterFile->nextEvent(m_storageController.keepEvent()) && 
                            (eventLimit_ < 0 || (n_events_processed) < eventLimit_)) {
                        // clean up for storage control calculation
                        m_storageController.resetEventState();
            
                        // notify for new run if necessary
                        if (theEvent.getEventHeader()->getRun() != wasRun) {
                            wasRun = theEvent.getEventHeader()->getRun();
                            try {
                                const RunHeader& runHeader = masterFile->getRunHeader(wasRun);
                                std::cout << "[ Process ] : got new run header from '" << masterFile->getFileName() << "' ..." << std::endl;
                                runHeader.Print();
                                for (auto module : sequence_) {
                                    module->onNewRun(runHeader);
                                }
                            } catch (const Exception&) {
                                std::cout << "[ Process ] : [WARNING] Run header for run " << wasRun << " was not found!" << std::endl;
                            }
                        }

                        if ( (logFrequency_ != -1) && ((n_events_processed + 1)%logFrequency_ == 0)) { 
                            TTimeStamp t;
                            std::cout << "[ Process ] :  Processing " << n_events_processed + 1 
                                      << " Run " << theEvent.getEventHeader()->getRun() 
                                      << " Event " << theEvent.getEventHeader()->getEventNumber() 
                                      << "  (" << t.AsString("lc") << ")" << std::endl;
                        }

                        for (auto module : sequence_) {
                            if (dynamic_cast<Producer*>(module)) {
                                (dynamic_cast<Producer*>(module))->produce(theEvent);
                            } else if (dynamic_cast<Analyzer*>(module)) {
                                (dynamic_cast<Analyzer*>(module))->analyze(theEvent);
                            }
                        }

                        n_events_processed++;
                    } //loop through events

                    if (eventLimit_ > 0 && n_events_processed == eventLimit_) {
                        std::cout << "[ Process ] : Reached event limit of " << eventLimit_ << " events\n";
                    }

                    if (eventLimit_ == 0 && n_events_processed > eventLimit_) {
                        std::cout << "[ Process ] : Processing interrupted\n";
                    }

                    if ( outFile and !singleOutput ) {
                        outFile->close();
                        delete outFile;
                        outFile = nullptr;
                    }

                    inFile.close();

                    std::cout << "[ Process ] : Closing file " << infilename << std::endl;

                    for (auto module : sequence_) {
                        module->onFileClose(infilename);
                    }

                } //loop through input files

                if ( outFile ) {
                    //close outFile
                    //  outFile would survive to here in single output mode
                    outFile->close();
                    delete outFile;
                    outFile = nullptr;
                }

                if (histoTFile_) {
                    histoTFile_->Write();
                    delete histoTFile_;
                    histoTFile_ = 0;
                }
            }

            // finally, notify everyone that we are stopping
            for (auto module : sequence_) {
                module->onProcessEnd();
            }
        } catch (Exception& e) {
            std::cerr << "Framework Error [" << e.name() << "] : " << e.message() << std::endl;
            std::cerr << "  at " << e.module() << ":" << e.line() << " in " << e.function() << std::endl;
        }
    }

    void Process::addToSequence(EventProcessor* mod) {
        sequence_.push_back(mod);
    }

    void Process::addFileToProcess(const std::string& filename) {
        inputFiles_.push_back(filename);
    }

    void Process::addDropKeepRule(const std::string& rule) {
        dropKeepRules_.push_back(rule);
    }

    void Process::setOutputFileName(const std::string& filenameOut) {
        outputFiles_.clear();
        outputFiles_.push_back(filenameOut);
    }

    void Process::setHistogramFileName(const std::string& filenameOut) {
        histoFilename_ = filenameOut;
    }

    void Process::addOutputFileName(const std::string& filenameOut) {
        outputFiles_.push_back(filenameOut);
    }

    TDirectory* Process::makeHistoDirectory(const std::string& dirName) {
        TDirectory* owner;
        if (histoFilename_.empty()) {
            owner = gROOT;
        } else if (histoTFile_ == 0) {
            histoTFile_ = new TFile(histoFilename_.c_str(), "RECREATE");
            owner = histoTFile_;
        } else
            owner = histoTFile_;
        owner->cd();
        TDirectory* child = owner->mkdir((char*) dirName.c_str());
        if (child)
            child->cd();
        return child;
    }
}
