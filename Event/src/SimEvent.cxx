#include "Event/SimEvent.h"

ClassImp(event::SimEvent)

using event::EventConstants;

namespace event {

SimEvent::SimEvent() :
        simParticles_     (new TClonesArray(EventConstants::SIM_PARTICLE.c_str(),        EventConstants::DEFAULT_COLLECTION_SIZE)),
        taggerSimHits_    (new TClonesArray(EventConstants::SIM_TRACKER_HIT.c_str(),     EventConstants::DEFAULT_COLLECTION_SIZE)),
        recoilSimHits_    (new TClonesArray(EventConstants::SIM_TRACKER_HIT.c_str(),     EventConstants::DEFAULT_COLLECTION_SIZE)),
        ecalSimHits_      (new TClonesArray(EventConstants::SIM_CALORIMETER_HIT.c_str(), EventConstants::DEFAULT_COLLECTION_SIZE)),
        hcalSimHits_      (new TClonesArray(EventConstants::SIM_CALORIMETER_HIT.c_str(), EventConstants::DEFAULT_COLLECTION_SIZE)),
        targetSimHits_    (new TClonesArray(EventConstants::SIM_CALORIMETER_HIT.c_str(), EventConstants::DEFAULT_COLLECTION_SIZE)),
        triggerPadSimHits_(new TClonesArray(EventConstants::SIM_CALORIMETER_HIT.c_str(), EventConstants::DEFAULT_COLLECTION_SIZE)) {

    collMap_[EventConstants::RECOIL_SIM_HITS] = recoilSimHits_;
    collMap_[EventConstants::TAGGER_SIM_HITS] = taggerSimHits_;
    collMap_[EventConstants::SIM_PARTICLES] = simParticles_;
    collMap_[EventConstants::ECAL_SIM_HITS] = ecalSimHits_;
    collMap_[EventConstants::HCAL_SIM_HITS] = hcalSimHits_;
    collMap_[EventConstants::TRIGGER_PAD_SIM_HITS] = triggerPadSimHits_;
    collMap_[EventConstants::TARGET_SIM_HITS] = targetSimHits_;
}

void SimEvent::Print(Option_t*) const {
    std::cout << std::endl;
    std::cout << "SimEvent " << eventNumber_ << std::endl;
    std::cout << "  " << EventConstants::SIM_PARTICLES        << ": " << simParticles_->GetEntries() << std::endl;
    std::cout << "  " << EventConstants::RECOIL_SIM_HITS      << ": " << recoilSimHits_->GetEntries() << std::endl;
    std::cout << "  " << EventConstants::TAGGER_SIM_HITS      << ": " << taggerSimHits_->GetEntries() << std::endl;
    std::cout << "  " << EventConstants::ECAL_SIM_HITS        << ": " << ecalSimHits_->GetEntries() << std::endl;
    std::cout << "  " << EventConstants::HCAL_SIM_HITS        << ": " << hcalSimHits_->GetEntries() << std::endl;
    std::cout << "  " << EventConstants::TRIGGER_PAD_SIM_HITS << ": " << triggerPadSimHits_->GetEntries() << std::endl;
    std::cout << "  " << EventConstants::TARGET_SIM_HITS      << ": " << targetSimHits_->GetEntries() << std::endl;
    std::cout << std::endl;
}


}
