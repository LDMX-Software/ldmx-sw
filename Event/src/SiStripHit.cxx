
#include "Event/SiStripHit.h"

ClassImp(ldmx::SiStripHit)

namespace ldmx { 

    SiStripHit::SiStripHit() 
        : RawHit() {  
    }

    SiStripHit::~SiStripHit() {
        Clear();
    }

    void SiStripHit::Print(std::ostream& o) const { 
        o << "SiStripHit {" 
          << "ADC Values: [ ";  
        for (auto const& adcValue : adcValues_) { 
            o << adcValue;
            if (&adcValue == &adcValues_.back()) o << "]";
            else o << ", ";
        }
        o << ", Time: " << time_ << "}";
    }

    void SiStripHit::Clear() { 
        adcValues_.clear(); 
        time_ = -9999;
        simTrackerHits_.clear();
    }

    void SiStripHit::addSimTrackerHit(const SimTrackerHit hit) { 
        simTrackerHits_.push_back( hit );
    }
}
