
#include "Event/SiStripHit.h"

ClassImp(ldmx::SiStripHit)

namespace ldmx { 

    SiStripHit::SiStripHit() 
        : RawHit() {  
    }

    SiStripHit::~SiStripHit() {
        Clear();
    }

    void SiStripHit::Print(Option_t* option) const { 
        
        std::cout << "[ SiStripHit ]:\n" 
                  << "\t ADC Values: [ ";  
        for (auto const& adcValue : adcValues_) { 
            std::cout << adcValue << ", "; 
        }
        std::cout << " ]\n"  
                  << "\t Time: " << time_ << std::endl;

    }

    void SiStripHit::Clear(Option_t* option) { 
        TObject::Clear();  
        adcValues_.clear(); 
        time_ = -9999;
        simTrackerHits_.clear();
    }

    void SiStripHit::addSimTrackerHit(const SimTrackerHit hit) { 
        simTrackerHits_.push_back( hit );
    }
}
