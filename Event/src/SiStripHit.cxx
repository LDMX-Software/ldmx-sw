
#include "Event/SiStripHit.h"

ClassImp(ldmx::SiStripHit)

namespace ldmx { 

    SiStripHit::SiStripHit() 
        : RawHit() {  
    }

    SiStripHit::~SiStripHit() {
        Clear();
    }

    void SiStripHit::Print() const { 
        
        std::cout << "[ SiStripHit ]:\n" 
                  << "\t ADC Values: [ ";  
        for (auto const& adcValue : adcValues_) { 
            std::cout << adcValue << ", "; 
        }
        std::cout << " ]\n"  
                  << "\t Time: " << time_ << std::endl;

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
