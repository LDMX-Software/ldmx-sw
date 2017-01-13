#include "EventProc/EventLoop.h"

// STL
#include <iostream>
#include <stdexcept>

namespace eventproc {

void EventLoop::initialize() {

    if (inputFileName_ == "") {
        throw std::runtime_error("Input file name was not set.");
    }
    std::cout << "[ EventLoop ] : Setting up input file " << inputFileName_ << std::endl;
    inputFile_ = new event::EventFile(inputFileName_, event::EventConstants::EVENT_TREE_NAME, false, 9);

    if (outputFileName_ == "") {
        throw std::runtime_error("Output file name was not set.");
    }
    std::cout << "[ EventLoop ] - Setting up output file " << outputFileName_ << std::endl;
    outputFile_ = new event::EventFile(outputFileName_, inputFile_);

    if (inputFileName_ == outputFileName_) {
        throw std::runtime_error("Input and output file names are the same!");
    }

    // Setup the EventImpl object.
    eventImpl_ = new event::EventImpl(passName_);
    outputFile_->setupEvent(eventImpl_);

    // Create event wrapper for access by the processors.
    event_ = new event::Event(eventImpl_);

    // Initialize the processors.
    for (EventProcessor* processor : processors_) {
        processor->setEvent(event_);
        processor->initialize(); 
    }
}

void EventLoop::run(int nEvents) {
    int nProcessed = 0;
    while (outputFile_->nextEvent()) {
        for (EventProcessor* processor : processors_) {
            processor->execute(); 
        }
        ++nProcessed;
        if (nEvents > 0 && nProcessed >= nEvents) {
            break;
        }
    }
    std::cout << "EventLoop: Finished processing " << nProcessed << " events out of "
            << nEvents << " requested." << std::endl;
}

void EventLoop::finish() {

    for (EventProcessor* processor : processors_) {
       processor->finish(); 
    }

    outputFile_->close();

    delete outputFile_;
    delete event_;
    delete eventImpl_;
}

}
