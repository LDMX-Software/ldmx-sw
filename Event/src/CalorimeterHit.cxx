#include "Event/CalorimeterHit.h"

// STL
#include <iostream>

ClassImp(event::CalorimeterHit)

namespace event {

    void CalorimeterHit::Clear(Option_t *option) {

	TObject::Clear();


	id_ = 0;
	amplitude_ = 0;
	energy_ = 0;
	time_ = 0;
    }

    void CalorimeterHit::Print(Option_t *option)  const {
	std::cout << "CalorimeterHit { " << "id: " << id_ << ",  energy: " << energy_
		  << "MeV, time: " << time_ << "ns, amplitude: " << amplitude_ << std::endl;
	
    }
}
