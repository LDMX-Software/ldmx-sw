#include "SimCore/Event/SimParticle.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::SimParticle);

namespace ldmx {
SimParticle::ProcessTypeMap SimParticle::createProcessTypeMap() {
  ProcessTypeMap proc_map;
  /// Electromagnetic interactions
  /// e Z --> e Z gamma
  proc_map["eBrem"] = ProcessType::eBrem;
  /// gamma --> e+ e-
  proc_map["conv"] = ProcessType::conv;
  /// e+ e- --> gamma gamma
  proc_map["annihil"] = ProcessType::annihil;
  /// gamma e --> gamma e
  proc_map["compt"] = ProcessType::compt;
  /// gamma Z --> e- Z
  proc_map["phot"] = ProcessType::phot;
  /// Electron ionization
  proc_map["eIoni"] = ProcessType::eIoni;
  /// Multiple scattering
  proc_map["msc"] = ProcessType::msc;
  /// gamma Z --> Z + X
  proc_map["photonNuclear"] = ProcessType::photonNuclear;
  /// mu Z --> Z + X
  proc_map["muonNuclear"] = ProcessType::muonNuclear;
  /// e Z --> e Z + X
  proc_map["electronNuclear"] = ProcessType::electronNuclear;
  /// gamma --> mu+ mu-
  proc_map["GammaToMuPair"] = ProcessType::GammaToMuPair;
  /// e- Z --> e- Z A'
  proc_map["DarkBrem"] = ProcessType::eDarkBrem;

  // Inelastic interactions
  /// n + Z -> X
  proc_map["neutronInelastic"] = ProcessType::neutronInelastic;
  /// n + Z -> Z*
  proc_map["neutronCapture"] = ProcessType::neutronCapture;
  /// K + Z -> X
  proc_map["kaon-Inelastic"] = ProcessType::kaonInelastic;
  proc_map["kaon+Inelastic"] = ProcessType::kaonInelastic;
  proc_map["kaon0LInelastic"] = ProcessType::kaonInelastic;
  proc_map["kaon0SInelastic"] = ProcessType::kaonInelastic;
  /// pi + Z -> X
  proc_map["pi-Inelastic"] = ProcessType::pionInelastic;
  proc_map["pi+Inelastic"] = ProcessType::pionInelastic;
  /// p + Z -> X
  proc_map["protonInelastic"] = ProcessType::protonInelastic;

  /// Other
  /// Primary particle
  proc_map["Primary"] = ProcessType::Primary;
  // Decay
  proc_map["Decay"] = ProcessType::Decay;
  return proc_map;
}

SimParticle::ProcessTypeMap SimParticle::process_map =
    SimParticle::createProcessTypeMap();

void SimParticle::clear() {
  daughters_.clear();
  parents_.clear();

  energy_ = 0;
  pdg_id_ = 0;
  gen_status_ = -1;
  time_ = 0;
  vtx_x_ = 0;
  vtx_y_ = 0;
  vtx_z_ = 0;
  end_x_ = 0;
  end_y_ = 0;
  end_z_ = 0;
  px_ = 0;
  py_ = 0;
  pz_ = 0;
  end_px_ = 0;
  end_py_ = 0;
  end_pz_ = 0;
  mass_ = 0;
  charge_ = 0;
  process_type_ = ProcessType::unknown;
  vertex_volume_ = "";
}

std::ostream& operator<<(std::ostream& o, const SimParticle& sp) {
  return o << "SimParticle { " << "energy: " << sp.energy_ << ", "
           << "pdg_id: " << sp.pdg_id_ << ", "
           << "gen_status: " << sp.gen_status_ << ", " << "time: " << sp.time_
           << ", " << "vertex: ( " << sp.vtx_x_ << ", " << sp.vtx_y_ << ", "
           << sp.vtx_z_ << " ), " << "end_point: ( " << sp.end_x_ << ", "
           << sp.end_y_ << ", " << sp.end_z_ << " ), " << "momentum: ( "
           << sp.px_ << ", " << sp.py_ << ", " << sp.pz_ << " ), "
           << "end_point_momentum: ( " << sp.end_px_ << ", " << sp.end_py_
           << ", " << sp.end_pz_ << " ), " << "mass: " << sp.mass_ << ", "
           << "n_daughters: " << sp.daughters_.size() << ", "
           << "n_parents: " << sp.parents_.size() << ", "
           << "process_type: " << sp.process_type_ << ", "
           << "vertex_volume: " << sp.vertex_volume_ << " }";
}

SimParticle::ProcessType SimParticle::findProcessType(
    std::string process_name) {
  if (process_name.find("biasWrapper") != std::string::npos) {
    std::size_t pos = process_name.find_first_of("(") + 1;
    process_name = process_name.substr(pos, process_name.size() - pos - 1);
  }

  if (process_map.find(process_name) != process_map.end()) {
    return process_map[process_name];
  } else {
    return ProcessType::unknown;
  }
}

void SimParticle::encodeTracks(
    std::function<int(int, unsigned int, unsigned int)> encodeFunc,
    const unsigned int encoding_version, const unsigned int event_index) {
  for (auto& daughter_track : this->daughters_) {
    daughter_track = encodeFunc(daughter_track, encoding_version, event_index);
  }  // over daughters
  for (auto& parent_track : this->parents_) {
    parent_track = encodeFunc(parent_track, encoding_version, event_index);
  }  // over parents
}

}  // namespace ldmx
