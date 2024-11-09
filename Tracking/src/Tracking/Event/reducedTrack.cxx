#include "Tracking/Event/reducedTrack.h"

#include <iostream>

ClassImp(ldmx::reducedTrack)

    namespace ldmx {
  void reducedTrack::Print() const { std::cout << "print track" << std::endl; }
}
