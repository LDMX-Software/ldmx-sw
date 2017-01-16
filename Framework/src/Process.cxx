#include <iostream>
#include "Framework/EventProcessor.h"
#include "Framework/EventImpl.h"
#include "Framework/EventFile.h"
#include "Framework/Process.h"

namespace ldmxsw {

  Process::Process(const std::string& passname) : passname_{passname} {
  }


  void Process::run(int eventlimit) {
    try {
      int n_events_processed=0;
    
      // first, notify everyone that we are starting
      for (auto module : sequence_)
	module->onProcessStart();
      
      // if we have no input files, but do have an event number, run for that number of events on an output file
      if (inputFiles_.empty() && eventlimit>0) {
	EventFile outFile(outputFileNameRule_,true);
	
	for (auto module : sequence_)
	  module->onFileOpen();
	
	EventImpl theEvent(passname_);
	outFile.setupEvent(&theEvent);
	
	while (n_events_processed<eventlimit) {
	  event::EventHeader& eh=theEvent.getEventHeaderMutable();
	  eh.setRun(runForGeneration_);
	  eh.setEventNumber(n_events_processed+1);
	  eh.setTimestamp(TTimeStamp());

	  theEvent.getEventHeader()->Print();
	  
	  for (auto module : sequence_)
	    if (dynamic_cast<Producer*>(module)) (dynamic_cast<Producer*>(module))->produce(theEvent);
	    else if (dynamic_cast<Analyzer*>(module)) (dynamic_cast<Analyzer*>(module))->analyze(theEvent);

	  outFile.nextEvent();
	  theEvent.Clear();
	  n_events_processed++;
	}
	
	for (auto module : sequence_)	  
	  module->onFileClose();

	outFile.close();
      } else {    
	// next, loop through the files
	for (auto infilename : inputFiles_) {
	  EventFile inFile(infilename);
	  EventFile* outFile(0);
	  
	  if (!outputFileNameRule_.empty()) {
	    outFile=new EventFile(outputFileNameRule_,&inFile);

	    for (auto rule : dropKeepRules_)
	      outFile->addDrop(rule);
	  }
	  
	  for (auto module : sequence_)
	    module->onFileOpen();
	  
	  EventImpl theEvent(passname_);
	  if (outFile) outFile->setupEvent(&theEvent);
	  else inFile.setupEvent(&theEvent);

	  EventFile* masterFile=(outFile)?(outFile):(&inFile);
	  
	  while (masterFile->nextEvent()) {	      
	    for (auto module : sequence_)
	      if (dynamic_cast<Producer*>(module)) (dynamic_cast<Producer*>(module))->produce(theEvent);
	      else if (dynamic_cast<Analyzer*>(module)) (dynamic_cast<Analyzer*>(module))->analyze(theEvent);
	    n_events_processed++;
	    theEvent.Clear();
	  }

	  if (outFile) outFile->close();
	  inFile.close();
	  
	  for (auto module : sequence_)	  
	    module->onFileClose();
	}
      }
      
      // finally, notify everyone that we are stopping
      for (auto module : sequence_)
	module->onProcessEnd();
    } catch (Exception& e) {
      std::cerr << "Framework Error [" << e.name() << "] : " << e.message() << std::endl;
      std::cerr << "  at " << e.module() <<":"<<e.line()<<" in " <<e.function() << std::endl;
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
  void Process::setOutputFileTemplate(const std::string& filenameOut) {
    outputFileNameRule_=filenameOut;
  }
  
}
