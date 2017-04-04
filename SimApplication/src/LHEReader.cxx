#include "SimApplication/LHEReader.h"

// STL
#include <iostream>
#include <stdexcept>

namespace ldmx {

LHEReader::LHEReader(std::string& filename) {
    std::cout << "Opening LHE file " << filename << std::endl;
    ifs_.open(filename.c_str(), std::ifstream::in);
}

LHEReader::~LHEReader() {
    ifs_.close();
}

LHEEvent* LHEReader::readNextEvent() {

    std::string line;
    bool foundEventElement = false;
    while (getline(ifs_, line)) {
        if (line == "<event>") {
            foundEventElement = true;
            break;
        }
    }

    if (!foundEventElement) {
        std::cerr << "WARNING: No next <event> element was found by the LHE reader." << std::endl;
        return NULL;
    }

    getline(ifs_, line);

    LHEEvent* nextEvent = new LHEEvent(line);

    while (getline(ifs_, line)) {

        if (line == "</event>") {
            break;
        }

        LHEParticle* particle = new LHEParticle(line);
        nextEvent->addParticle(particle);
    }

    const std::vector<LHEParticle*>& particles = nextEvent->getParticles();
    int particleIndex = 0;
    for (std::vector<LHEParticle*>::const_iterator it = particles.begin();
            it != particles.end(); it++) {
        LHEParticle* particle = (*it);
        if (particle->getMOTHUP(0) != 0) {
            int mother1 = particle->getMOTHUP(0);
            int mother2 = particle->getMOTHUP(1);
            if (mother1 > 0) {
                particle->setMother(0, particles[mother1 - 1]);
            }
            if (mother2 > 0) {
                particle->setMother(1, particles[mother2 - 1]);
            }
        }
        ++particleIndex;
    }

    return nextEvent;
}

}
