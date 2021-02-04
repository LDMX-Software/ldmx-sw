#include "SimCore/LHEParticle.h"
#include "Framework/Exception/Exception.h"

// STL
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>

// Geant4
#include "globals.hh"

namespace simcore {

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

  idup_ = atof(tokens[0].c_str());
  istup_ = atoi(tokens[1].c_str());
  mothup_[0] = atoi(tokens[2].c_str());
  mothup_[1] = atoi(tokens[3].c_str());
  icolup_[0] = atoi(tokens[4].c_str());
  icolup_[1] = atoi(tokens[5].c_str());
  pup_[0] = atof(tokens[6].c_str());
  pup_[1] = atof(tokens[7].c_str());
  pup_[2] = atof(tokens[8].c_str());
  pup_[3] = atof(tokens[9].c_str());
  pup_[4] = atof(tokens[10].c_str());
  vtimup_ = atof(tokens[11].c_str());
  spinup_ = atof(tokens[12].c_str());

  mothers_[0] = NULL;
  mothers_[1] = NULL;
}

int LHEParticle::getIDUP() const { return idup_; }

int LHEParticle::getISTUP() const { return istup_; }

int LHEParticle::getMOTHUP(int i) const { return mothup_[i]; }

int LHEParticle::getICOLUP(int i) const { return icolup_[i]; }

double LHEParticle::getPUP(int i) const { return pup_[i]; }

double LHEParticle::getVTIMUP() const { return vtimup_; }

double LHEParticle::getSPINUP() const { return spinup_; }

void LHEParticle::setMother(int i, LHEParticle* mother) {
  mothers_[i] = mother;
}

LHEParticle* LHEParticle::getMother(int i) const { return mothers_[i]; }

void LHEParticle::print(std::ostream& stream) const {
  stream << "LHEParticle { "
         << "IDUP: " << getIDUP() << ", ISTUP: " << getISTUP()
         << ", MOTHUP[0]: " << getMOTHUP(0) << ", MOTHUP[1]: " << getMOTHUP(1)
         << ", ICOLUP[0]: " << getICOLUP(0) << ", ICOLUP[1]: " << getICOLUP(1)
         << ", PUP[0]: " << getPUP(0) << ", PUP[1]: " << getPUP(1)
         << ", PUP[2]: " << getPUP(2) << ", PUP[3]: " << getPUP(3)
         << ", PUP[4]: " << getPUP(4) << ", VTIMUP: " << getVTIMUP()
         << ", SPINUP: " << getSPINUP() << " }" << std::endl;
}

std::ostream& operator<<(std::ostream& stream, const LHEParticle& particle) {
  particle.print(stream);
  return stream;
}

}  // namespace simcore
