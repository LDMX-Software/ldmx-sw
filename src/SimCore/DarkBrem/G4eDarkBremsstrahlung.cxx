/**
 * @file G4eDarkBremsstrahlung.cxx
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/DarkBrem/G4eDarkBremsstrahlung.h"
#include "SimCore/DarkBrem/DarkBremVertexLibraryModel.h"
#include "SimCore/DarkBrem/G4APrime.h"

#include "Framework/RunHeader.h"

#include "G4Electron.hh"      //for electron definition
#include "G4EventManager.hh"  //for EventID number
#include "G4ProcessTable.hh"  //for deactivating dark brem process
#include "G4ProcessType.hh"   //for type of process
#include "G4RunManager.hh"    //for VerboseLevel

namespace ldmx {
namespace darkbrem {

G4double ElementXsecCache::get(G4double energy, G4double A, G4double Z) {
  key_t key = computeKey(energy, A, Z);
  if (the_cache_.find(key) == the_cache_.end()) {
    if (model_.get() == nullptr) {
      EXCEPTION_RAISE("BadCache",
                      "ElementXsecCache not given a model to calculate cross "
                      "sections with.");
    }
    the_cache_[key] = model_->ComputeCrossSectionPerAtom(energy, A, Z);
  }
  return the_cache_.at(key);
}

void ElementXsecCache::stream(std::ostream& o) const {
  o << "A [au],Z [protons],Energy [MeV],Xsec [pb]\n"
    << std::setprecision(std::numeric_limits<double>::digits10 +
                         1);  // maximum precision
  for (auto const& [key, xsec] : the_cache_) {
    key_t E = key % MAX_E;
    key_t A = ((key - E) / MAX_E) % MAX_A;
    key_t Z = ((key - E) / MAX_E - A) / MAX_A;
    o << A << "," << Z << "," << E << "," << xsec / CLHEP::picobarn << "\n";
  }
  o << std::endl;
}

ElementXsecCache::key_t ElementXsecCache::computeKey(G4double energy,
                                                     G4double A,
                                                     G4double Z) const {
  key_t energyKey = energy;
  key_t AKey = A;
  key_t ZKey = Z;
  return (ZKey * MAX_A + AKey) * MAX_E + energyKey;
}

const std::string G4eDarkBremsstrahlung::PROCESS_NAME = "eDarkBrem";

G4eDarkBremsstrahlung::G4eDarkBremsstrahlung(const Parameters& params)
    : G4VDiscreteProcess(G4eDarkBremsstrahlung::PROCESS_NAME,
                         fElectromagnetic) {
  // we need to pretend to be an EM process so the biasing framework recognizes
  // us
  SetProcessSubType(63);  // needs to be different from the other Em Subtypes

  only_one_per_event_ = params.getParameter<bool>("only_one_per_event");
  cache_xsec_ = params.getParameter<bool>("cache_xsec");
  ap_mass_ = params.getParameter<double>("ap_mass");

  auto model{params.getParameter<Parameters>("model")};
  auto model_name{model.getParameter<std::string>("name")};
  if (model_name == "vertex_library") {
    model_ = std::make_shared<DarkBremVertexLibraryModel>(model);
  } else {
    EXCEPTION_RAISE("DarkBremModel",
                    "Model named '" + model_name + "' is not known.");
  }

  // now that the model is set, calculate common xsec and put them into the
  // cache
  if (cache_xsec_) {
    element_xsec_cache_ =
        ElementXsecCache(model_);  // remake cache with model attached
    CalculateCommonXsec();         // calculate common cross sections
  }
}

G4bool G4eDarkBremsstrahlung::IsApplicable(const G4ParticleDefinition& p) {
  return &p == G4Electron::Electron();
}

void G4eDarkBremsstrahlung::PrintInfo() {
  G4cout << " Only One Per Event               : " << only_one_per_event_
         << G4endl;
  G4cout << " A' Mass [MeV]                    : " << ap_mass_ << G4endl;
  model_->PrintInfo();
}

void G4eDarkBremsstrahlung::RecordConfig(RunHeader& h) const {
  h.setIntParameter("Only One DB Per Event", only_one_per_event_);
  h.setFloatParameter("A' Mass [MeV]", ap_mass_);
  model_->RecordConfig(h);
}

G4VParticleChange* G4eDarkBremsstrahlung::PostStepDoIt(const G4Track& track,
                                                       const G4Step& step) {
  // Debugging Purposes: Check if track we get is an electron
  if (not IsApplicable(*track.GetParticleDefinition()))
    EXCEPTION_RAISE(
        "DBBadTrack",
        "Dark brem process receieved a track that isn't applicable.");

  /*
   * Geant4 has decided that it is our time to interact,
   * so we are going to change the particle
   */
  ldmx_log(debug) << "A dark brem occurred!";

  if (only_one_per_event_) {
    // Deactivate the process after one dark brem if we restrict ourselves to
    // only one per event. If this is in the stepping action instead, more than
    // one brem can occur within each step. Reactivated in
    // RunManager::TerminateOneEvent Both biased and unbiased process could be in
    // the run (but not at the same time),
    //  so we turn off both while silencing the warnings from the process table.
    std::vector<G4String> db_process_name_options = {
        "biasWrapper(" + PROCESS_NAME + ")", PROCESS_NAME};
    G4ProcessTable* ptable = G4ProcessTable::GetProcessTable();
    G4int verbosity = ptable->GetVerboseLevel();
    ptable->SetVerboseLevel(0);
    for (auto const& name : db_process_name_options)
      ptable->SetProcessActivation(name, false);
    ptable->SetVerboseLevel(verbosity);
  }

  aParticleChange.Initialize(track);

  model_->GenerateChange(aParticleChange, track, step);

  /*
   * Parent class has some internal counters that need to be reset,
   * so we call it before returning. It will return our shared
   * protected member variable aParticleChange that we have been modifying
   */
  return G4VDiscreteProcess::PostStepDoIt(track, step);
}

void G4eDarkBremsstrahlung::CalculateCommonXsec() {
  // first in pair is A, second is Z
  std::vector<std::pair<G4double, G4double>> elements = {
      std::make_pair(183.84, 74),  // tungsten
      std::make_pair(28.085, 14)   // silicon
  };

  G4double current_energy = 2.0 * GeV;
  G4double maximum_energy = 4.0 * GeV;
  G4double energy_step = 1.0 * MeV;
  while (current_energy <= maximum_energy) {
    for (auto const& [A, Z] : elements)
      element_xsec_cache_.get(current_energy, A, Z);
    current_energy += energy_step;
  }
}

G4double G4eDarkBremsstrahlung::GetMeanFreePath(const G4Track& track, G4double,
                                                G4ForceCondition*) {
  // won't happen if it isn't applicable
  if (not IsApplicable(*track.GetParticleDefinition())) return DBL_MAX;

  G4Material* materialWeAreIn = track.GetMaterial();
  const G4ElementVector* theElementVector = materialWeAreIn->GetElementVector();
  const G4double* NbOfAtomsPerVolume =
      materialWeAreIn->GetVecNbOfAtomsPerVolume();

  G4double SIGMA = 0;
  for (size_t i = 0; i < materialWeAreIn->GetNumberOfElements(); i++) {
    G4double AtomicZ = (*theElementVector)[i]->GetZ();
    G4double AtomicA = (*theElementVector)[i]->GetA() / (g / mole);
    G4double energy = track.GetDynamicParticle()->GetKineticEnergy();

    G4double element_xsec;

    if (cache_xsec_)
      element_xsec = element_xsec_cache_.get(energy, AtomicA, AtomicZ);
    else
      element_xsec =
          model_->ComputeCrossSectionPerAtom(energy, AtomicA, AtomicZ);

    SIGMA += NbOfAtomsPerVolume[i] * element_xsec;
  }

  return SIGMA > DBL_MIN ? 1. / SIGMA : DBL_MAX;
}
}  // namespace darkbrem
}  // namespace ldmx
