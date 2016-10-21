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
    std::cout << "  " << Event::SIM_PARTICLES << ": " << event->getCollection(Event::SIM_PARTICLES)->GetEntries() << std::endl;
    std::cout << "  " << Event::RECOIL_SIM_HITS << ": " << event->getCollection(Event::RECOIL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << "  " << Event::TAGGER_SIM_HITS << ": " << event->getCollection(Event::TAGGER_SIM_HITS)->GetEntries() << std::endl;
    std::cout << "  " << Event::ECAL_SIM_HITS << ": " << event->getCollection(Event::ECAL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << "  " << Event::HCAL_SIM_HITS << ": " << event->getCollection(Event::HCAL_SIM_HITS)->GetEntries() << std::endl;
    ++nProcessed;
}

void DummyEventProcessor::finish() {
    std::cout << "DummyEventProcessor: finished processing "
            << nProcessed << " events " << std::endl;
}

}
