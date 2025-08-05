#include "SimCore/Event/SimParticle.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::SimParticle)

    namespace ldmx {
  SimParticle::ProcessTypeMap SimParticle::createProcessTypeMap() {
    ProcessTypeMap procMap;
    /// Electromagnetic interactions
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
    /// mu Z --> Z + X
    procMap["muonNuclear"] = ProcessType::muonNuclear;
    /// e Z --> e Z + X
    procMap["electronNuclear"] = ProcessType::electronNuclear;
    /// gamma --> mu+ mu-
    procMap["GammaToMuPair"] = ProcessType::GammaToMuPair;
    /// e- Z --> e- Z A'
    procMap["DarkBrem"] = ProcessType::eDarkBrem;

    // Inelastic interactions
    /// n + Z -> X
    procMap["neutronInelastic"] = ProcessType::neutronInelastic;
    /// n + Z -> Z*
    procMap["neutronCapture"] = ProcessType::neutronCapture;
    /// K + Z -> X
    procMap["kaon-Inelastic"] = ProcessType::kaonInelastic;
    procMap["kaon+Inelastic"] = ProcessType::kaonInelastic;
    procMap["kaon0LInelastic"] = ProcessType::kaonInelastic;
    procMap["kaon0SInelastic"] = ProcessType::kaonInelastic;
    /// pi + Z -> X
    procMap["pi-Inelastic"] = ProcessType::pionInelastic;
    procMap["pi+Inelastic"] = ProcessType::pionInelastic;
    /// p + Z -> X
    procMap["protonInelastic"] = ProcessType::protonInelastic;

    /// Other
    /// Primary particle
    procMap["Primary"] = ProcessType::Primary;
    // Decay
    procMap["Decay"] = ProcessType::Decay;
    return procMap;
  }

  SimParticle::ProcessTypeMap SimParticle::PROCESS_MAP =
      SimParticle::createProcessTypeMap();

  void SimParticle::clear() {
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

  std::ostream& operator<<(std::ostream& o, const SimParticle& sp) {
    return o << "SimParticle { " << "energy: " << sp.energy_ << ", "
             << "PDG ID: " << sp.pdgID_ << ", "
             << "genStatus: " << sp.genStatus_ << ", " << "time: " << sp.time_
             << ", " << "vertex: ( " << sp.x_ << ", " << sp.y_ << ", " << sp.z_
             << " ), " << "endPoint: ( " << sp.endX_ << ", " << sp.endY_ << ", "
             << sp.endZ_ << " ), " << "momentum: ( " << sp.px_ << ", " << sp.py_
             << ", " << sp.pz_ << " ), " << "endPointMomentum: ( " << sp.endpx_
             << ", " << sp.endpy_ << ", " << sp.endpz_ << " ), "
             << "mass: " << sp.mass_ << ", "
             << "nDaughters: " << sp.daughters_.size() << ", "
             << "nParents: " << sp.parents_.size() << ", "
             << "processType: " << sp.processType_ << ", "
             << "vertex volume: " << sp.vertexVolume_ << " }";
  }

  SimParticle::ProcessType SimParticle::findProcessType(
      std::string processName) {
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
}  // namespace ldmx
