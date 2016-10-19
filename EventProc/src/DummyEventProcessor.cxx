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
    std::cout << "  " << Event::SIM_PARTICLES << ": " << event->getCollectionSize(Event::SIM_PARTICLES) << std::endl;
    std::cout << "  " << Event::RECOIL_SIM_HITS << ": " << event->getCollectionSize(Event::RECOIL_SIM_HITS) << std::endl;
    std::cout << "  " << Event::TAGGER_SIM_HITS << ": " << event->getCollectionSize(Event::TAGGER_SIM_HITS) << std::endl;
    std::cout << "  " << Event::ECAL_SIM_HITS << ": " << event->getCollectionSize(Event::ECAL_SIM_HITS) << std::endl;
    std::cout << "  " << Event::HCAL_SIM_HITS << ": " << event->getCollectionSize(Event::HCAL_SIM_HITS) << std::endl;
    ++nProcessed;
}

void DummyEventProcessor::finish() {
    std::cout << "DummyEventProcessor: finished processing "
            << nProcessed << " events " << std::endl;
}

}
