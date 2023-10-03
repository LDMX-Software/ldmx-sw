#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"

namespace dqm {

/**
 * @class DarkBremInteraction
 *
 * Go through the particle map and find the dark brem products,
 * storing their vertex and the dark brem outgoing kinematics
 * for further study.
 *
 * ## Products
 * APrime{Px,Py,Pz} - 3-vector momentum of A' at dark brem
 * APrimeEnergy   - energy of A' at dark brem
 * Recoil{Px,Py,Pz} - 3-vector momentum of electron recoiling from dark brem
 * RecoilEnergy   - energy of recoil at dark brem
 * Incident{Px,Py,Pz} - 3-vector momentum of electron incident to dark brem
 * IncidentEnergy   - energy of incident electron at dark brem
 * APrimeParentID - TrackID of A' parent
 * DarkBremVertexMaterial - integer corresponding to index of known_materials 
 *                          parameter OR -1 if not found in known_materials
 * DarkBremVertexMaterialZ - elemental Z value for element chosen by random from
 *                           the elements in the material
 * DarkBrem{X,Y,Z} - physical space location where dark brem occurred
 */
class DarkBremInteraction : public framework::Producer {
 public:
  DarkBremInteraction(const std::string& n, framework::Process& p)
    : framework::Producer(n,p) {}
  virtual void onProcessStart() final override;
  virtual void produce(framework::Event& e) final override;
 private:
  /**
   * Set the labels of the histogram of the input name with the input labels
   */
  void setHistLabels(const std::string& name, const std::vector<std::string>& labels);
  /**
   * the list of known materials assiging them to material ID numbers
   *
   * During the simulation, we can store the name of the logical volume
   * that the particle originated in. There can be many copies of logical
   * volumes in different places but they all will be the same material
   * by construction of how we designed our GDML. In the ecal GDML, the
   * beginning the 'volume' tags list the logical volumes and you can
   * see there which materials they all are in.
   *
   * We go through this list on each event, checking if any of these entries
   * match a substring of the logical volume name stored. If we don't find any,
   * the integer ID is set to -1.
   *
   * The inverse LUT that can be used on the plotting side is
   * 
   *    material_lut = {
   *      0 : 'Unknown',
   *      1 : 'C',
   *      2 : 'PCB',
   *      3 : 'Glue',
   *      4 : 'Si',
   *      5 : 'Al',
   *      6 : 'W',
   *      7 : 'PVT'
   *    }
   *
   * This is kind of lazy, we could instead do a full LUT where we list all known
   * logical volume names and their associated materials but this analysis isn't
   * as important so I haven't invested that much time in it yet.
   */
  std::map<std::string, int> known_materials_ = {
    { "Carbon", 1 },
    { "PCB", 2 }, // in v12, the motherboards were simple rectangles with 'PCB' in the name
    { "Glue", 3 },
    { "Si", 4 },
    { "Al", 5 },
    { "W" , 6 },
    { "target", 6 },
    { "trigger_pad", 7 },
    { "strongback" , 5 }, // strongback is made of aluminum
    { "motherboard" , 2 }, // motherboards are PCB
    { "support" , 5 }, // support box is aluminum
    { "CFMix" , 3 }, // in v12, we called the Glue layers CFMix
    { "C_volume" , 1 } // in v12, we called the carbon cooling planes C but this is too general for substr matching
  };

  /**
   * The list of known elements assigning them to the bins that we are putting them into.
   *
   * There are two failure modes for this:
   * 1. The dark brem didn't happen, in which case, the element reported by the event header
   *    will be -1. We give this an ID of 0.
   * 2. The dark brem occurred within an element not listed here, in which case we give it
   *    the last bin.
   *
   * The inverset LUT that can be used if studying the output tree is
   *
   *    element_lut = {
   *      0 : 'did_not_happen',
   *      1 : 'H 1',
   *      2 : 'C 6',
   *      3 : 'O 8',
   *      4 : 'Na 11',
   *      5 : 'Si 14',
   *      6 : 'Ca 20',
   *      7 : 'Cu 29',
   *      8 : 'W 74',
   *      9 : 'unlisted'
   *    }
   */
  std::map<int, int> known_elements_ = {
    {1, 1},
    {6, 2},
    {8, 3},
    {11, 4},
    {14, 5},
    {20, 6},
    {29, 7},
    {74, 8}
  };
};

/**
 * calculate total energy from 3-momentum and mass
 *
 * @param[in] p 3-momentum
 * @param[in] m mass
 * @return total energy
 */
static double energy(const std::vector<double>& p, const double& m) {
  return sqrt(p.at(0)*p.at(0)+ p.at(1)*p.at(1)+ p.at(2)*p.at(2)+ m*m);
}

/**
 * calculate the sum in quadrature of the passed list of doubles
 */
static double quadsum(const std::initializer_list<double>& list) {
  double sum{0};
  for (const double& elem : list) sum += elem*elem;
  return sqrt(sum);
}


void DarkBremInteraction::setHistLabels(
    const std::string& name,
    const std::vector<std::string>& labels) {
  auto h{histograms_.get(name)};
  for (std::size_t ibin{1}; ibin <= labels.size(); ibin++) {
    h->GetXaxis()->SetBinLabel(ibin, labels[ibin-1].c_str());
  }
}

/**
 * update the labels of some categorical histograms
 */
void DarkBremInteraction::onProcessStart() {
  setHistLabels(
      "dark_brem_material",
      {
        "Unknown",
        "C",
        "PCB",
        "Glue",
        "Si",
        "Al",
        "W",
        "PVT"
      });

  setHistLabels(
      "dark_brem_element",
      {
        "did not happen",
        "H 1",
        "C 6",
        "O 8",
        "Na 11",
        "Si 14",
        "Ca 20",
        "Cu 29",
        "W 74",
        "unlisted"
      });
}

/**
 * extract the kinematics of the dark brem interaction from the SimParticles
 *
 * Sometimes the electron that undergoes the dark brem is not in a region 
 * where it should be saved (i.e. it is a shower electron inside of the ECal).
 * In this case, we need to reconstruct the incident momentum from the outgoing
 * products (the recoil electron and the dark photon) which should be saved by
 * the biasing filter used during the simulation.
 *
 * Since the dark brem model does not include a nucleus, it only is able to 
 * conserve momentum, so we need to reconstruct the incident particle's 3-momentum
 * and then use the electron mass to calculate its total energy.
 */
void DarkBremInteraction::produce(framework::Event& event) {
  histograms_.setWeight(event.getEventHeader().getWeight());
  const auto& particle_map{event.getMap<int,ldmx::SimParticle>("SimParticles")};
  const ldmx::SimParticle *recoil{nullptr}, *aprime{nullptr}, *beam{nullptr};
  for (const auto& [track_id, particle] : particle_map) {
    if (track_id == 1) beam = &particle;
    if (particle.getProcessType() == ldmx::SimParticle::ProcessType::eDarkBrem) {
      if (particle.getPdgID() == 622) {
        if (aprime != nullptr) {
          EXCEPTION_RAISE("BadEvent", "Found multiple A' in event.");
        }
        aprime = &particle;
      } else {
        recoil = &particle;
      }
    }
  }

  if (recoil == nullptr and aprime == nullptr) {
    /* dark brem did not occur during the simulation
     *    IF PROPERLY CONFIGURED, this occurs because the simulation
     *    exhausted the maximum number of tries to get a dark brem
     *    to occur. We just leave early so that the entries in the
     *    ntuple are the unphysical numeric minimum.
     *
     * This can also happen during development, so I leave a debug
     * printout here to be uncommented when developing the dark
     * brem simulation.
    std::cout << "Event " << e.getEventNumber()
      << " did not have a dark brem occur within it." << std::endl;
     */
    return;
  }

  if (recoil == nullptr or aprime == nullptr or beam == nullptr) {
    // we are going to end processing so let's take our time to
    // construct a nice error message
    std::stringstream err_msg;
    err_msg 
      << "Unable to final all necessary particles for DarkBrem interaction."
      << " Missing: [ "
      << (recoil == nullptr ? "recoil " : "")
      << (aprime == nullptr ? "aprime " : "")
      << (beam == nullptr ? "beam " : "")
      << "]" << std::endl;
    EXCEPTION_RAISE("BadEvent", err_msg.str());
    return;
  }

  const auto& recoil_p = recoil->getMomentum();
  const auto& aprime_p = aprime->getMomentum();

  std::vector<double> incident_p = recoil_p;
  for (std::size_t i{0}; i < recoil_p.size(); ++i) incident_p[i] += aprime_p.at(i);

  double incident_energy = energy(incident_p, recoil->getMass());
  double recoil_energy = energy(recoil_p, recoil->getMass());
  double visible_energy = (beam->getEnergy() - incident_energy) + recoil_energy;

  std::vector<double> ap_vertex{aprime->getVertex()}; 
  std::string ap_vertex_volume{aprime->getVertexVolume()};
  auto ap_vertex_material_it = std::find_if(
      known_materials_.begin(), known_materials_.end(),
      [&](const auto& mat_pair) {
        return ap_vertex_volume.find(mat_pair.first) != std::string::npos;
      }
      );
  int ap_vertex_material = (ap_vertex_material_it != known_materials_.end()) ?
                            ap_vertex_material_it->second : 0;

  int ap_parent_id{-1};
  if (aprime->getParents().size() > 0) {
    ap_parent_id = aprime->getParents().at(0);
  } else {
    ldmx_log(error) << "Found A' without a parent ID!";
  }

  float aprime_energy = energy(aprime_p, aprime->getMass());
  int aprime_genstatus = aprime->getGenStatus();
  double aprime_px{aprime_p.at(0)}, aprime_py{aprime_p.at(1)}, aprime_pz{aprime_p.at(2)};
  event.add("APrimeEnergy", aprime_energy);
  event.add("APrimePx", aprime_px);
  event.add("APrimePy", aprime_py);
  event.add("APrimePz", aprime_pz);
  event.add("APrimeParentID", ap_parent_id);
  event.add("APrimeGenStatus", aprime_genstatus);

  histograms_.fill("aprime_energy", aprime_energy);
  histograms_.fill("aprime_pt", quadsum({aprime_px, aprime_py}));

  int recoil_genstatus = recoil->getGenStatus();
  double recoil_px{recoil_p.at(0)}, recoil_py{recoil_p.at(1)}, recoil_pz{recoil_p.at(2)};
  event.add("RecoilEnergy", recoil_energy);
  event.add("RecoilPx", recoil_px);
  event.add("RecoilPy", recoil_py);
  event.add("RecoilPz", recoil_pz);
  event.add("RecoilGenStatus", recoil_genstatus);

  histograms_.fill("recoil_energy", recoil_energy);
  histograms_.fill("recoil_pt", quadsum({recoil_px, recoil_py}));

  event.add("IncidentEnergy", incident_energy);
  double incident_px{incident_p.at(0)}, incident_py{incident_p.at(1)}, incident_pz{incident_p.at(2)};
  event.add("IncidentPx", incident_px);
  event.add("IncidentPy", incident_py);
  event.add("IncidentPz", incident_pz);

  histograms_.fill("incident_energy", incident_energy);
  histograms_.fill("incident_pt", quadsum({incident_px, incident_py}));

  double vtx_x{aprime->getVertex().at(0)},
         vtx_y{aprime->getVertex().at(1)},
         vtx_z{aprime->getVertex().at(2)};
  event.add("DarkBremX", vtx_x);
  event.add("DarkBremY", vtx_y);
  event.add("DarkBremZ", vtx_z);
  event.add("DarkBremVertexMaterial", ap_vertex_material);
  float db_material_z = event.getEventHeader().getFloatParameter("db_material_z");
  event.add("DarkBremVertexMaterialZ", db_material_z);

  histograms_.fill("dark_brem_z", vtx_z);

  int i_element = 0;
  if (db_material_z > 0) {
    if (known_elements_.find(static_cast<int>(db_material_z)) == known_elements_.end()) {
      i_element = known_elements_.size();
    } else {
      i_element = known_elements_.at(static_cast<int>(db_material_z));
    }
  }

  histograms_.fill("dark_brem_element", i_element);
  histograms_.fill("dark_brem_material", ap_vertex_material);
}

}

DECLARE_ANALYZER_NS(dqm,DarkBremInteraction);
