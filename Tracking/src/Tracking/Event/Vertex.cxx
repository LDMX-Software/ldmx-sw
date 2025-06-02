#include "Tracking/Event/Vertex.h"

#include <iostream>

ClassImp(ldmx::Vertex)

    namespace ldmx {
  void Vertex::Print() const { std::cout << "print vertex" << std::endl; }
}
