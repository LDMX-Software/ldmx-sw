#include "SimCore/LHE/LHEEvent.h"

namespace simcore {
namespace lhe {

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

  // Number of particles in the event
  nup_ = atoi(tokens[0].c_str());
  // The physics process ID
  idprup_ = atoi(tokens[1].c_str());
  // The event weight
  xwgtup_ = atof(tokens[2].c_str());
  // Scale Q of parton distributions
  scalup_ = atof(tokens[3].c_str());
  // QED coupling value
  aqedup_ = atof(tokens[4].c_str());
  // QCD coupling value
  aqcdup_ = atof(tokens[5].c_str());

  vtx_[0] = 0;
  vtx_[1] = 0;
  vtx_[2] = 0;
}

int LHEEvent::getNUP() const { return nup_; }

int LHEEvent::getIDPRUP() const { return idprup_; }

double LHEEvent::getXWGTUP() const { return xwgtup_; }

double LHEEvent::getSCALUP() const { return scalup_; }

double LHEEvent::getAQEDUP() const { return aqedup_; }

double LHEEvent::getAQCDUP() const { return aqcdup_; }

const double* LHEEvent::getVertex() const { return vtx_; }

double LHEEvent::getVertexTime() const { return vtxt_; }

void LHEEvent::addParticle(std::unique_ptr<LHEParticle> particle) {
  particles_.push_back(std::move(particle));
}

const std::vector<std::unique_ptr<LHEParticle>>& LHEEvent::getParticles()
    const {
  return particles_;
}

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

}  // namespace lhe
}  // namespace simcore
