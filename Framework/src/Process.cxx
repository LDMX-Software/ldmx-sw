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
        passname_ { passname } {
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

                for (auto module : sequence_) {
                    if (dynamic_cast<Producer*>(module)) {
                        (dynamic_cast<Producer*>(module))->produce(theEvent);
                    } else if (dynamic_cast<Analyzer*>(module)) {
                        (dynamic_cast<Analyzer*>(module))->analyze(theEvent);
                    }
                }
                outFile.nextEvent();
                theEvent.Clear();
                n_events_processed++;
            }

            for (auto module : sequence_) {
                module->onFileClose(outputFiles_[0]);
            }
            outFile.close();

        } else {
            if (!outputFiles_.empty() && outputFiles_.size() != inputFiles_.size()) {
                EXCEPTION_RAISE("Process", "Unable to handle case of different number of input and output files (other than zero output files)");
            }
            // next, loop through the files
            int ifile = 0;
            int wasRun = -1;
            for (auto infilename : inputFiles_) {
                EventFile inFile(infilename);

                std::cout << "Process: Opening file " << infilename << std::endl;
                EventFile* outFile(0);

                if (!outputFiles_.empty()) {
                    outFile = new EventFile(outputFiles_[ifile], &inFile);
                    ifile++;

                    for (auto rule : dropKeepRules_) {
                        outFile->addDrop(rule);
                    }
                }

                for (auto module : sequence_) {
                    module->onFileOpen(infilename);
                }

                EventImpl theEvent(passname_);
                if (outFile) {
                    outFile->setupEvent(&theEvent);
                } else {
                    inFile.setupEvent(&theEvent);
                }
                EventFile* masterFile = (outFile) ? (outFile) : (&inFile);

                while (masterFile->nextEvent() && (eventLimit_ < 0 || (n_events_processed) < eventLimit_)) {
                    // notify for new run if necessary
                    if (theEvent.getEventHeader()->getRun() != wasRun) {
                        wasRun = theEvent.getEventHeader()->getRun();
                        try {
                            const RunHeader& runHeader = inFile.getRunHeader(wasRun);
                            runHeader.Print();
                            for (auto module : sequence_) {
                                module->onNewRun(runHeader);
                            }
                        } catch (const Exception&) {
                            std::cout << "Process: WARNING - Run header for run " << wasRun << " was not found in the input file." << std::endl;
                        }
                    }

                    TTimeStamp t;
                    std::cout << "[ Process ] :  Processing " << n_events_processed + 1 << " Run " << theEvent.getEventHeader()->getRun() << " Event " << theEvent.getEventHeader()->getEventNumber() << "  (" << t.AsString("lc") << ")" << std::endl;

                    for (auto module : sequence_) {
                        if (dynamic_cast<Producer*>(module)) {
                            (dynamic_cast<Producer*>(module))->produce(theEvent);
                        } else if (dynamic_cast<Analyzer*>(module)) {
                            (dynamic_cast<Analyzer*>(module))->analyze(theEvent);
                        }
                    }
                    n_events_processed++;

                }
                if (eventLimit_ > 0 && n_events_processed == eventLimit_) {
                    std::cout << "[ Process ] : Reached event limit of " << eventLimit_ << " events\n";
                }

                if (outFile) {
                    outFile->close();
                }
                inFile.close();
                std::cout << "Process: Closing file " << infilename << std::endl;
                for (auto module : sequence_) {
                    module->onFileClose(infilename);
                }
            }

	    if (histoTFile_) {
		histoTFile_->Write();
		delete histoTFile_;
		histoTFile_=0;
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
	histoFilename_=filenameOut;
    }
    void Process::addOutputFileName(const std::string& filenameOut) {
	outputFiles_.push_back(filenameOut);
    }

    TDirectory* Process::makeHistoDirectory(const std::string& dirName) {
	TDirectory* owner;
	if (histoFilename_.empty()) {
	    owner=gROOT;
	} else if (histoTFile_==0) {
	    histoTFile_=new TFile(histoFilename_.c_str(),"RECREATE");
	    owner=histoTFile_;
	} else owner=histoTFile_;
	owner->cd();
	TDirectory* child=owner->mkdir((char*)dirName.c_str());
	if (child) child->cd();
	return child;	
    }
  
}
