#include "SimApplication/LHEEvent.h"

// Geant4
#include "globals.hh"

// STL
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace sim {

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

    nup_ = atoi(tokens[0].c_str());
    idprup_ = atoi(tokens[1].c_str());
    xwgtup_ = atof(tokens[2].c_str());
    scalup_ = atof(tokens[3].c_str());
    aqedup_ = atof(tokens[4].c_str());
    aqcdup_ = atof(tokens[5].c_str());
}

LHEEvent::~LHEEvent() {
    for (std::vector<LHEParticle*>::iterator it = particles_.begin();
            it != particles_.end(); it++) {
        delete (*it);
    }
    particles_.clear();
}

int LHEEvent::getNUP() {
    return nup_;
}

int LHEEvent::getIDPRUP() {
    return idprup_;
}

double LHEEvent::getXWGTUP() {
    return xwgtup_;
}

double LHEEvent::getSCALUP() {
    return scalup_;
}

double LHEEvent::getAQEDUP() {
    return aqedup_;
}

double LHEEvent::getAQCDUP() {
    return aqcdup_;
}

void LHEEvent::addParticle(LHEParticle* particle) {
    particles_.push_back(particle);
}

const std::vector<LHEParticle*>& LHEEvent::getParticles() {
    return particles_;
}

}
