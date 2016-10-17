#include "SimApplication/LHEParticle.h"

// STL
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>

// Geant4
#include "globals.hh"

LHEParticle::LHEParticle(std::string& line) {

    std::istringstream iss(line);
    std::vector<std::string> tokens;
    do {
        std::string elem;
        iss >> elem;
        if (elem.size() != 0) {
            tokens.push_back(elem);
        }
    } while (iss);

    if (tokens.size() != 13) {
        std::cerr << "ERROR: Bad particle record in LHE file ..." << std::endl;
        std::cerr << "  " << line << std::endl;
        G4Exception("LHEParticle::LHEParticle", "LHEParticleError", FatalException, "Wrong number of tokens in LHE particle record.");
    }

    idup = atof(tokens[0].c_str());
    istup = atoi(tokens[1].c_str());
    mothup[0] = atoi(tokens[2].c_str());
    mothup[1] = atoi(tokens[3].c_str());
    icolup[0] = atoi(tokens[4].c_str());
    icolup[1] = atoi(tokens[5].c_str());
    pup[0] = atof(tokens[6].c_str());
    pup[1] = atof(tokens[7].c_str());
    pup[2] = atof(tokens[8].c_str());
    pup[3] = atof(tokens[9].c_str());
    pup[4] = atof(tokens[10].c_str());
    vtimup = atof(tokens[11].c_str());
    spinup = atof(tokens[12].c_str());

    mothers[0] = NULL;
    mothers[1] = NULL;
}

int LHEParticle::getIDUP() const {
    return idup;
}

int LHEParticle::getISTUP() const {
    return istup;
}

int LHEParticle::getMOTHUP(int i) const {
    return mothup[i];
}

int LHEParticle::getICOLUP(int i) const {
    return icolup[i];
}

double LHEParticle::getPUP(int i) const {
    return pup[i];
}

double LHEParticle::getVTIMUP() const {
    return vtimup;
}

double LHEParticle::getSPINUP() const {
    return spinup;
}

void LHEParticle::setMother(int i, LHEParticle* mother) {
    mothers[i] = mother;
}

LHEParticle* LHEParticle::getMother(int i) const {
    return mothers[i];
}

void LHEParticle::print(std::ostream& stream) const {
    stream << "LHEParticle { "
            << "IDUP: " << getIDUP()
            << ", ISTUP: " << getISTUP()
            << ", MOTHUP[0]: " << getMOTHUP(0)
            << ", MOTHUP[1]: " << getMOTHUP(1)
            << ", ICOLUP[0]: " << getICOLUP(0)
            << ", ICOLUP[1]: " << getICOLUP(1)
            << ", PUP[0]: " << getPUP(0)
            << ", PUP[1]: " << getPUP(1)
            << ", PUP[2]: " << getPUP(2)
            << ", PUP[3]: " << getPUP(3)
            << ", PUP[4]: " << getPUP(4)
            << ", VTIMUP: " << getVTIMUP()
            << ", SPINUP: " << getSPINUP()
            << " }"
            << std::endl;
}

std::ostream& operator <<(std::ostream& stream, const LHEParticle& particle) {
    particle.print(stream);
    return stream;
}
