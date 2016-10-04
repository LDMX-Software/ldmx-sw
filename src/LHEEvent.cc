#include "SimApplication/LHEEvent.h"

// Geant4
#include "globals.hh"

// STL
#include <iostream>
#include <sstream>
#include <stdexcept>

LHEEvent::LHEEvent(std::string& line) {

    std::istringstream iss(line);
    std::vector<std::string> tokens;
    do {
        std::string elem;
        iss >> elem;
        if (elem.size() != 0) {
            tokens.push_back(elem);
        }
    } while (iss);

    if (tokens.size() != 6) {
        std::cerr << "ERROR: Bad event information record in LHE file ..." << std::endl;
        std::cerr << "  " << line << std::endl;
        G4Exception("LHEEvent::LHEEvent", "LHEEventError", FatalException, "Wrong number of tokens in LHE event information record.");
    }

    nup = atoi(tokens[0].c_str());
    idprup = atoi(tokens[1].c_str());
    xwgtup = atof(tokens[2].c_str());
    scalup = atof(tokens[3].c_str());
    aqedup = atof(tokens[4].c_str());
    aqcdup = atof(tokens[5].c_str());
}

LHEEvent::~LHEEvent() {
}

int LHEEvent::getNUP() {
    return nup;
}

int LHEEvent::getIDPRUP() {
    return idprup;
}

double LHEEvent::getXWGTUP() {
    return xwgtup;
}

double LHEEvent::getSCALUP() {
    return scalup;
}

double LHEEvent::getAQEDUP() {
    return aqedup;
}

double LHEEvent::getAQCDUP() {
    return aqcdup;
}

void LHEEvent::addParticle(LHEParticle* particle) {
    particles.push_back(particle);
}

const std::vector<LHEParticle*>& LHEEvent::getParticles() {
    return particles;
}
