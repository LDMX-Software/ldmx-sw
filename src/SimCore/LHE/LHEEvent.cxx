#include "SimCore/LHEEvent.h"
#include "Framework/Exception/Exception.h"

// Geant4
#include "globals.hh"

// STL
#include <iostream>
#include <sstream>

namespace simcore {

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
    EXCEPTION_RAISE("TokenNum",
                    "Wrong number of tokens in LHE event information record.");
  }

  nup_ = atoi(tokens[0].c_str());
  idprup_ = atoi(tokens[1].c_str());
  xwgtup_ = atof(tokens[2].c_str());
  scalup_ = atof(tokens[3].c_str());
  aqedup_ = atof(tokens[4].c_str());
  aqcdup_ = atof(tokens[5].c_str());

  vtx_[0] = 0;
  vtx_[1] = 0;
  vtx_[2] = 0;
}

LHEEvent::~LHEEvent() {
  for (std::vector<LHEParticle*>::iterator it = particles_.begin();
       it != particles_.end(); it++) {
    delete (*it);
  }
  particles_.clear();
}

int LHEEvent::getNUP() const { return nup_; }

int LHEEvent::getIDPRUP() const { return idprup_; }

double LHEEvent::getXWGTUP() const { return xwgtup_; }

double LHEEvent::getSCALUP() const { return scalup_; }

double LHEEvent::getAQEDUP() const { return aqedup_; }

double LHEEvent::getAQCDUP() const { return aqcdup_; }

const double* LHEEvent::getVertex() const { return vtx_; }

const double LHEEvent::getVertexTime() const { return vtxt_; }

void LHEEvent::addParticle(LHEParticle* particle) {
  particles_.push_back(particle);
}

const std::vector<LHEParticle*>& LHEEvent::getParticles() { return particles_; }

void LHEEvent::setVertex(double x, double y, double z) {
  vtx_[0] = x;
  vtx_[1] = y;
  vtx_[2] = z;
}

/**
 * Parse the vertex from a line of the form "#vertex [x] [y] [z] [t]"
 * Where [t] is assumed zero if not specified
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

  if (tokens.size() != 4 && tokens.size() != 5) {
    EXCEPTION_RAISE("TokenNum",
                    "Wrong number of tokens or format in LHE event vertex "
                    "information record.");
  }
  vtx_[0] = atof(tokens[1].c_str());
  vtx_[1] = atof(tokens[2].c_str());
  vtx_[2] = atof(tokens[3].c_str());
  if (tokens.size() > 4) {
    vtxt_ = atof(tokens[4].c_str());
  }
}

}  // namespace simcore
