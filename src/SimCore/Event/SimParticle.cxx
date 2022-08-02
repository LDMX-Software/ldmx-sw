#include "SimCore/Event/SimParticle.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::SimParticle)

namespace ldmx {
SimParticle::ProcessTypeMap SimParticle::createProcessTypeMap() {
  ProcessTypeMap procMap;
  /// e Z --> e Z gamma
  procMap["eBrem"] = ProcessType::eBrem;
  /// gamma --> e+ e-
  procMap["conv"] = ProcessType::conv;
  /// e+ e- --> gamma gamma
  procMap["annihil"] = ProcessType::annihil;
  /// gamma e --> gamma e
  procMap["compt"] = ProcessType::compt;
  /// gamma Z --> e- Z
  procMap["phot"] = ProcessType::phot;
  /// Electron ionization
  procMap["eIoni"] = ProcessType::eIoni;
  /// Multiple scattering
  procMap["msc"] = ProcessType::msc;
  /// gamma Z --> Z + X
  procMap["photonNuclear"] = ProcessType::photonNuclear;
  /// e Z --> e Z + X
  procMap["electronNuclear"] = ProcessType::electronNuclear;
  /// gamma --> mu+ mu-
  procMap["GammaToMuPair"] = ProcessType::GammaToMuPair;
  /// e- Z --> e- Z A'
  procMap["eDBrem"] = ProcessType::eDarkBrem;
  return procMap;
}

SimParticle::ProcessTypeMap SimParticle::PROCESS_MAP =
    SimParticle::createProcessTypeMap();

SimParticle::SimParticle() {}

SimParticle::~SimParticle() {}

void SimParticle::Clear() {
  daughters_.clear();
  parents_.clear();

  energy_ = 0;
  pdgID_ = 0;
  genStatus_ = -1;
  time_ = 0;
  x_ = 0;
  y_ = 0;
  z_ = 0;
  endX_ = 0;
  endY_ = 0;
  endZ_ = 0;
  px_ = 0;
  py_ = 0;
  pz_ = 0;
  endpx_ = 0;
  endpy_ = 0;
  endpz_ = 0;
  mass_ = 0;
  charge_ = 0;
  processType_ = ProcessType::unknown;
  vertexVolume_ = "";
}

void SimParticle::Print() const {
  std::cout << "SimParticle { "
            << "energy: " << energy_ << ", "
            << "PDG ID: " << pdgID_ << ", "
            << "genStatus: " << genStatus_ << ", "
            << "time: " << time_ << ", "
            << "vertex: ( " << x_ << ", " << y_ << ", " << z_ << " ), "
            << "endPoint: ( " << endX_ << ", " << endY_ << ", " << endZ_
            << " ), "
            << "momentum: ( " << px_ << ", " << py_ << ", " << pz_ << " ), "
            << "endPointMomentum: ( " << endpx_ << ", " << endpy_ << ", "
            << endpz_ << " ), "
            << "mass: " << mass_ << ", "
            << "nDaughters: " << daughters_.size() << ", "
            << "nParents: " << parents_.size() << ", "
            << "processType: " << processType_ << ", "
            << "vertex volume: " << vertexVolume_ << " }" << std::endl;
}

SimParticle::ProcessType SimParticle::findProcessType(std::string processName) {
  if (processName.find("biasWrapper") != std::string::npos) {
    std::size_t pos = processName.find_first_of("(") + 1;
    processName = processName.substr(pos, processName.size() - pos - 1);
  }

  if (PROCESS_MAP.find(processName) != PROCESS_MAP.end()) {
    return PROCESS_MAP[processName];
  } else {
    return ProcessType::unknown;
  }
}
} // namespace ldmx
