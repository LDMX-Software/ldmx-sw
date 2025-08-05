#include "SimCore/Event/SimParticle.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::SimParticle);

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

  SimParticle::ProcessTypeMap SimParticle::process_map_ =
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
         << ", " << "vertex: ( " << sp.vtx_x_ << ", " << sp.vtx_y_ << ", " << sp.vtx_z_
         << " ), " << "end_point: ( " << sp.end_x_ << ", " << sp.end_y_ << ", "
         << sp.end_z_ << " ), " << "momentum: ( " << sp.px_ << ", " << sp.py_
         << ", " << sp.pz_ << " ), " << "end_point_momentum: ( " << sp.end_px_
         << ", " << sp.end_py_ << ", " << sp.end_pz_ << " ), "
         << "mass: " << sp.mass_ << ", "
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

        if (process_map_.find(process_name) != process_map_.end()) {
          return process_map_[process_name];
        } else {
          return ProcessType::unknown;
        }
      }
}  // namespace ldmx
