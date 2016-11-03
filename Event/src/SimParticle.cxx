#include "Event/SimParticle.h"

// STL
#include <iostream>

ClassImp(event::SimParticle)

namespace event {

SimParticle::SimParticle()
    : daughters(new TRefArray()), parents(new TRefArray())
{;}

SimParticle::~SimParticle() {
    daughters->Delete();
    parents->Delete();
}

void SimParticle::Print(Option_t *option) const {
    std::cout << "SimParticle { " <<
            "energy: " << energy << ", " <<
            "pdg: " << pdg << ", " <<
            "genStatus: " << genStatus << ", " <<
            "time: " << time << ", " <<
            "vertex: ( " << x << ", " << y << ", " << z << " ), " <<
            "endPoint: ( " << endX << ", " << endY << ", " << endZ << " ), " <<
            "momentum: ( " << px << ", " << py << ", " << pz << " ), " <<
            "mass: " << mass << ", " <<
            "nDaughters: " << daughters->GetEntries() << ", "
            "nParents: " << parents->GetEntries() <<
            " }" << std::endl;
}


std::vector<SimParticle*> SimParticle::getDaughters() {
    std::vector<SimParticle*> dauVec;
    for (int iDau = 0; iDau < daughters->GetEntries(); iDau++) {
        dauVec.push_back((SimParticle*) daughters->At(iDau));
    }
    return dauVec;
}

std::vector<SimParticle*> SimParticle::getParents() {
    std::vector<SimParticle*> parVec;
    for (int iParent = 0; iParent < parents->GetEntries(); iParent++) {
        parVec.push_back((SimParticle*) parents->At(iParent));
    }
    return parVec;
}

}

