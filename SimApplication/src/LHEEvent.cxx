#include "SimApplication/LHEEvent.h"

// Geant4
#include "globals.hh"

// STL
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace ldmx {

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
<<<<<<< HEAD

	vtx_[0]=0;
	vtx_[1]=0;
	vtx_[2]=0;
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    }

    LHEEvent::~LHEEvent() {
        for (std::vector<LHEParticle*>::iterator it = particles_.begin(); it != particles_.end(); it++) {
            delete (*it);
        }
        particles_.clear();
    }

<<<<<<< HEAD
    int LHEEvent::getNUP() const {
        return nup_;
    }

    int LHEEvent::getIDPRUP() const {
        return idprup_;
    }

    double LHEEvent::getXWGTUP() const {
        return xwgtup_;
    }

    double LHEEvent::getSCALUP() const {
        return scalup_;
    }

    double LHEEvent::getAQEDUP() const {
        return aqedup_;
    }

    double LHEEvent::getAQCDUP() const {
        return aqcdup_;
    }

    const double* LHEEvent::getVertex() const {
        return vtx_;
    }

=======
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

>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    void LHEEvent::addParticle(LHEParticle* particle) {
        particles_.push_back(particle);
    }

    const std::vector<LHEParticle*>& LHEEvent::getParticles() {
        return particles_;
    }

<<<<<<< HEAD
    void LHEEvent::setVertex(double x, double y, double z) {
        vtx_[0]=x;
        vtx_[1]=y;
        vtx_[2]=z;
    }
	
   /**
    * Parse the vertex from a line of the form "#vertex [x] [y] [z]"
    */
    void LHEEvent::setVertex(const std::string& line) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        do {
            std::string elem;
            iss >> elem;
            if (elem.size() != 0) {
                tokens.push_back(elem);
            }
        } while (iss);

        if (tokens.size() != 4 || tokens[0]!="#vertex") {
            std::cerr << "ERROR: Bad event vertex information record in LHE file ..." << std::endl;
            std::cerr << "  " << line << std::endl;
            G4Exception("LHEEvent::LHEEvent", "LHEEventError", FatalException, "Wrong number of tokens or format in LHE event vertex information record.");
        }
	vtx_[0]=atof(tokens[1].c_str());
	vtx_[1]=atof(tokens[2].c_str());
	vtx_[2]=atof(tokens[3].c_str());	
    }

=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
}
