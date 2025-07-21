#include "SimCore/LHE/LHEParticle.h"

namespace simcore {
namespace lhe {

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
    EXCEPTION_RAISE("TokenNum",
                    "Wrong number of tokens in LHE particle record.");
  }

  pdg_id_ = atof(tokens[0].c_str());
  status_ = atoi(tokens[1].c_str());
  mother_[0] = atoi(tokens[2].c_str());
  mother_[1] = atoi(tokens[3].c_str());
  color_[0] = atoi(tokens[4].c_str());
  color_[1] = atoi(tokens[5].c_str());
  momentum_[0] = atof(tokens[6].c_str());
  momentum_[1] = atof(tokens[7].c_str());
  momentum_[2] = atof(tokens[8].c_str());
  momentum_[3] = atof(tokens[9].c_str());
  momentum_[4] = atof(tokens[10].c_str());
  lifetime_ = atof(tokens[11].c_str());
  spin_ = atof(tokens[12].c_str());

  mothers_[0] = nullptr;
  mothers_[1] = nullptr;
}

int LHEParticle::getPdgId() const { return pdg_id_; }

int LHEParticle::getStatus() const { return status_; }

int LHEParticle::getMother(int i) const { return mother_[i]; }

int LHEParticle::getColor(int i) const { return color_[i]; }

double LHEParticle::getMomentum(int i) const { return momentum_[i]; }

double LHEParticle::getLifetime() const { return lifetime_; }

double LHEParticle::getSpin() const { return spin_; }

void LHEParticle::setMother(int i, LHEParticle* mother) {
  mothers_[i] = mother;
}

LHEParticle* LHEParticle::getMotherParticle(int i) const { return mothers_[i]; }

void LHEParticle::print() const {
  std::cout << "LHEParticle { " << "PDG ID: " << getPdgId()
            << ", Status: " << getStatus() << ", Mother[0]: " << getMother(0)
            << ", Mother[1]: " << getMother(1) << ", Color[0]: " << getColor(0)
            << ", Color[1]: " << getColor(1)
            << ", Momentum[0]: " << getMomentum(0)
            << ", Momentum[1]: " << getMomentum(1)
            << ", Momentum[2]: " << getMomentum(2)
            << ", Momentum[3]: " << getMomentum(3)
            << ", Momentum[4]: " << getMomentum(4)
            << ", Time: " << getLifetime() << ", Spin: " << getSpin() << " }"
            << std::endl;
}

}  // namespace lhe
}  // namespace simcore