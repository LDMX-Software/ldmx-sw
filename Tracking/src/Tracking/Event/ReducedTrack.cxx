#include "Tracking/Event/ReducedTrack.h"

#include <iostream>

ClassImp(ldmx::ReducedTrack)

namespace ldmx {
    void ReducedTrack::Print() const { std::cout << "print reduced track" << std::endl; }
}
