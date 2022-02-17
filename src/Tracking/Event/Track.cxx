#include "Tracking/Event/Track.h"

#include <iostream>

ClassImp(ldmx::Track)

namespace ldmx {

void Track::Print() const {
  std::cout<<"print track"<<std::endl;
}

}
