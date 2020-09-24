
#include "Tracking/Event/RawSiStripHit.h"

std::ostream& operator << (std::ostream& output, const RawSiStripHit& hit) {
    output << "[ RawSiStripHit ]: Samples: { "; 
    for(auto isample{0}; isample < (hit.samples_.size() - i); ++isample) 
       output << hit.samples_[isample] << ", "; 
    output << hit.samples_[hit.samples_.size() - 1] << " } " 
           << "Time: " << hit.time_ << std::endl;

    return output;  
} 
