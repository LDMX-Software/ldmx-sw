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

  num_particles_ = atoi(tokens[0].c_str());
  // The physics process ID
  process_id_ = atoi(tokens[1].c_str());
  // The event weight
  event_weight_ = atof(tokens[2].c_str());
  // Scale Q of parton distributions
  scale_q_ = atof(tokens[3].c_str());
  // QED coupling value
  coupling_qed_ = atof(tokens[4].c_str());
  // QCD coupling value
  coupling_qcd_ = atof(tokens[5].c_str());

  vtx_[0] = 0;
  vtx_[1] = 0;
  vtx_[2] = 0;
}

int LHEEvent::getNumParticles() const { return num_particles_; }

int LHEEvent::getProcessID() const { return process_id_; }

double LHEEvent::getEventWeight() const { return event_weight_; }

double LHEEvent::getScaleQ() const { return scale_q_; }

double LHEEvent::getCouplingQed() const { return coupling_qed_; }

double LHEEvent::getCouplingQcd() const { return coupling_qcd_; }

const double* LHEEvent::getVertex() const { return vtx_; }

double LHEEvent::getVertexTime() const { return vtxt_; }

void LHEEvent::addParticle(std::unique_ptr<LHEParticle> particle) {
  particles_.push_back(std::move(particle));
}

const std::vector<std::unique_ptr<LHEParticle>>& LHEEvent::getParticles()
    const {
  return particles_;
}

void LHEEvent::setVertex(double x_, double y_, double z_) {
  vtx_[0] = x_;
  vtx_[1] = y_;
  vtx_[2] = z_;
}

/**
 * Parse the vertex from a line of the form "#vertex [x_] [y_] [z_] [t]"
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
