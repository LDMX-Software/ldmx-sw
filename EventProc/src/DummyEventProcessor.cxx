#include "EventProc/DummyEventProcessor.h"

namespace eventproc {

void DummyEventProcessor::initialize() {
    std::cout << "DummyEventProcessor: initialize" << std::endl;
}

void DummyEventProcessor::execute() {
    Event* event = this->getEvent();
    std::cout << std::endl;
    std::cout << "DummyEventProcessor: read event " << event->getEventNumber() << std::endl;
    std::cout << std::endl;
    std::cout << "  " << event::SIM_PARTICLES << ": " << event->getCollection(event::SIM_PARTICLES)->GetEntries() << std::endl;
    std::cout << "  " << event::RECOIL_SIM_HITS << ": " << event->getCollection(event::RECOIL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << "  " << event::TAGGER_SIM_HITS << ": " << event->getCollection(event::TAGGER_SIM_HITS)->GetEntries() << std::endl;
    std::cout << "  " << event::ECAL_SIM_HITS << ": " << event->getCollection(event::ECAL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << "  " << event::HCAL_SIM_HITS << ": " << event->getCollection(event::HCAL_SIM_HITS)->GetEntries() << std::endl;
    ++nProcessed_;
}

void DummyEventProcessor::finish() {
    std::cout << "DummyEventProcessor: finished processing "
            << nProcessed_ << " events " << std::endl;
}

}
