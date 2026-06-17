/**
 * @file GenieElectroNuclearProcess.cxx
 * @brief Implementation of the GENIE-based electronuclear G4VDiscreteProcess.
 */

#include "SimCore/GenieElectroNuclearProcess.h"

#include "SimCore/G4User/UserEventInformation.h"

// GENIE
#include "Framework/Conventions/Units.h"
#include "Framework/EventGen/EventRecord.h"
#include "Framework/GHEP/GHepParticle.h"
#include "Framework/Interaction/InitialState.h"
#include "Framework/Numerical/RandomGen.h"
#include "Framework/Utils/AppInit.h"
#include "Framework/Utils/RunOpt.h"
#include "GENIE/Framework/Interaction/InitialState.h"
#include "GENIE/Framework/Utils/RunOpt.h"
#include "HepMC3/GenEvent.h"

// Geant4
#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4EventManager.hh"
#include "G4IonTable.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "Randomize.hh"

// ROOT
#include <TLorentzVector.h>
#include <TParticle.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>

namespace simcore {

const std::string GenieElectroNuclearProcess::PROCESS_NAME = "electronNuclear";

GenieElectroNuclearProcess::GenieElectroNuclearProcess(
    const framework::config::Parameters& params)
    : G4VDiscreteProcess(PROCESS_NAME, fElectromagnetic) {
  // Use a distinct EM subtype so we don't collide with other EM processes.
  // 64 is arbitrary but distinct from standard EM subtypes and the DarkBrem
  // subtype (63).
  SetProcessSubType(64);

  // Read configuration
  targets_ = params.get<std::vector<int>>("targets");
  abundances_ = params.get<std::vector<double>>("abundances");
  tune_ = params.get<std::string>("tune");
  spline_file_ = params.get<std::string>("spline_file");
  message_threshold_file_ = params.get<std::string>("message_threshold_file");
  only_one_per_event_ = params.get<bool>("only_one_per_event");

  // Normalize abundances
  double abundance_sum = 0;
  for (auto a : abundances_) abundance_sum += a;
  if (abundance_sum > 0) {
    for (auto& a : abundances_) a /= abundance_sum;
  }

  // Initialize GENIE and set up drivers
  initializeGENIE();
  setupDrivers();

  // Build the Z -> targets lookup map
  for (size_t i = 0; i < targets_.size(); ++i) {
    int Z = (targets_[i] / 10000) % 1000;
    z_to_targets_[Z].emplace_back(static_cast<int>(i), abundances_[i]);
  }

  ldmx_log(info) << "GenieElectroNuclearProcess configured with "
                 << targets_.size() << " target(s), tune=" << tune_;
}

GenieElectroNuclearProcess::~GenieElectroNuclearProcess() {
  std::cout << "--- GENIE Process Summary ---" << std::endl;
  for (size_t i = 0; i < targets_.size(); ++i) {
    std::cout << "  Target=" << targets_[i] << "  Abundance=" << abundances_[i]
              << std::endl;
  }
  std::cout << "--- GENIE Process Summary END ---" << std::endl;
}

void GenieElectroNuclearProcess::initializeGENIE() {
  // Initialize RunOpt with EM event generator list
  char* in_arr[3] = {const_cast<char*>(""),
                     const_cast<char*>("--event-generator-list"),
                     const_cast<char*>("EM")};
  genie::RunOpt::Instance()->ReadFromCommandLine(3, in_arr);

  // Set message thresholds
  genie::utils::app_init::MesgThresholds(message_threshold_file_);

  // Set tune
  genie::RunOpt::Instance()->SetTuneName(tune_);
  if (!genie::RunOpt::Instance()->Tune()) {
    EXCEPTION_RAISE("ConfigurationException", "No TuneId in RunOption.");
  }
  genie::RunOpt::Instance()->BuildTune();

  // Load cross-section spline file
  genie::utils::app_init::XSecTable(spline_file_, true);

  genie::GHepRecord::SetPrintLevel(0);
}

void GenieElectroNuclearProcess::setupDrivers() {
  evg_drivers_.resize(targets_.size());
  for (size_t i = 0; i < targets_.size(); ++i) {
    genie::InitialState initial_state(targets_[i], 11);  // electron probe
    evg_drivers_[i].SetEventGeneratorList(
        genie::RunOpt::Instance()->EventGeneratorList());
    evg_drivers_[i].SetUnphysEventMask(
        *genie::RunOpt::Instance()->UnphysEventMask());
    evg_drivers_[i].Configure(initial_state);
    evg_drivers_[i].UseSplines();
  }
}

G4bool GenieElectroNuclearProcess::IsApplicable(const G4ParticleDefinition& p) {
  return &p == G4Electron::Definition();
}

G4double GenieElectroNuclearProcess::GetMeanFreePath(
    const G4Track& track, G4double /*prevStepSize*/,
    G4ForceCondition* /*condition*/) {
  if (!IsApplicable(*track.GetParticleDefinition())) return DBL_MAX;

  // Get electron total energy in GeV (GENIE units)
  G4double energy_geant4 = track.GetDynamicParticle()->GetTotalEnergy();
  double energy_gev = energy_geant4 / CLHEP::GeV;

  if (energy_gev < 0.001) return DBL_MAX;  // threshold guard

  // Get direction from the track
  G4ThreeVector dir = track.GetMomentumDirection();

  // Build electron 4-momentum for GENIE
  double electron_mass_gev = 0.000510999;
  double elec_p = std::sqrt(energy_gev * energy_gev -
                            electron_mass_gev * electron_mass_gev);
  if (elec_p <= 0) return DBL_MAX;

  TLorentzVector e_p4(elec_p * dir.x(), elec_p * dir.y(), elec_p * dir.z(),
                      energy_gev);

  G4Material* material = track.GetMaterial();
  const G4ElementVector* elements = material->GetElementVector();
  const G4double* atomDensity = material->GetVecNbOfAtomsPerVolume();
  size_t n_elements = material->GetNumberOfElements();

  G4double SIGMA = 0;
  partial_sum_sigma_.resize(n_elements);

  for (size_t i = 0; i < n_elements; ++i) {
    int Z = static_cast<int>((*elements)[i]->GetZ());

    G4double element_sigma = 0;
    auto it = z_to_targets_.find(Z);
    if (it != z_to_targets_.end()) {
      for (const auto& [driver_idx, abundance] : it->second) {
        // Query GENIE for this target's cross section
        double xsec_genie = evg_drivers_[driver_idx].XSecSum(e_p4);
        // Convert from GENIE internal units to Geant4 units
        double xsec_g4 =
            (xsec_genie / genie::units::millibarn) * CLHEP::millibarn;
        element_sigma += abundance * xsec_g4;
      }
    }
    // else: no GENIE target for this Z, contributes 0

    SIGMA += atomDensity[i] * element_sigma;
    partial_sum_sigma_[i] = SIGMA;
  }

  return SIGMA > DBL_MIN ? 1.0 / SIGMA : DBL_MAX;
}

G4VParticleChange* GenieElectroNuclearProcess::PostStepDoIt(
    const G4Track& track, const G4Step& step) {
  // Lazy initialization of GENIE random seed on first event
  if (!genie_initialized_) {
    auto seed = G4Random::getTheEngine()->getSeed();
    ldmx_log(debug) << "Initializing GENIE random seed: " << seed;
    genie::utils::app_init::RandGen(seed);
    genie_initialized_ = true;
  }

  aParticleChange.Initialize(track);

  // If only one per event, deactivate this process
  if (only_one_per_event_) {
    G4ProcessManager* pman = track.GetDefinition()->GetProcessManager();
    for (int i_proc = 0; i_proc < pman->GetProcessList()->size(); i_proc++) {
      G4VProcess* p = (*(pman->GetProcessList()))[i_proc];
      if (p->GetProcessName().contains(PROCESS_NAME)) {
        pman->SetProcessActivation(p, false);
        break;
      }
    }
  }

  // Get electron energy at the post-step point
  G4double energy_geant4 = step.GetPostStepPoint()->GetTotalEnergy();
  double energy_gev = energy_geant4 / CLHEP::GeV;

  G4ThreeVector dir = track.GetMomentumDirection();

  ldmx_log(info) << "GENIE electronuclear interaction at E = " << energy_gev
                 << " GeV";

  // Build electron 4-momentum for GENIE
  double electron_mass_gev = 0.000510999;
  double elec_p = std::sqrt(energy_gev * energy_gev -
                            electron_mass_gev * electron_mass_gev);
  TLorentzVector e_p4(elec_p * dir.x(), elec_p * dir.y(), elec_p * dir.z(),
                      energy_gev);

  // Select target isotope via weighted random from partial sums
  G4Material* material = track.GetMaterial();
  const G4ElementVector* elements = material->GetElementVector();
  size_t n_elements = material->GetNumberOfElements();

  int selected_driver = -1;

  if (n_elements == 1) {
    // Only one element — pick from matching GENIE targets by abundance
    int Z = static_cast<int>((*elements)[0]->GetZ());
    auto it = z_to_targets_.find(Z);
    if (it != z_to_targets_.end() && !it->second.empty()) {
      if (it->second.size() == 1) {
        selected_driver = it->second[0].first;
      } else {
        // Weighted random among isotopes
        double total_ab = 0;
        for (const auto& [idx, ab] : it->second) total_ab += ab;
        double rand_val = G4UniformRand() * total_ab;
        double running = 0;
        for (const auto& [idx, ab] : it->second) {
          running += ab;
          if (rand_val <= running) {
            selected_driver = idx;
            break;
          }
        }
        if (selected_driver < 0) selected_driver = it->second.back().first;
      }
    }
  } else {
    // Multiple elements — use partial_sum_sigma_ to pick element first,
    // then pick isotope within that element
    double rand_val = G4UniformRand() * partial_sum_sigma_[n_elements - 1];
    int selected_element = static_cast<int>(n_elements) - 1;
    for (size_t i = 0; i < n_elements - 1; ++i) {
      if (rand_val <= partial_sum_sigma_[i]) {
        selected_element = static_cast<int>(i);
        break;
      }
    }

    int Z = static_cast<int>((*elements)[selected_element]->GetZ());
    auto it = z_to_targets_.find(Z);
    if (it != z_to_targets_.end() && !it->second.empty()) {
      if (it->second.size() == 1) {
        selected_driver = it->second[0].first;
      } else {
        // Weighted random among isotopes for this Z
        // Weight by abundance * xsec
        std::vector<double> weights;
        double total_w = 0;
        for (const auto& [idx, ab] : it->second) {
          double xsec = evg_drivers_[idx].XSecSum(e_p4);
          double w = ab * xsec;
          weights.push_back(w);
          total_w += w;
        }
        double r = G4UniformRand() * total_w;
        double running = 0;
        for (size_t j = 0; j < it->second.size(); ++j) {
          running += weights[j];
          if (r <= running) {
            selected_driver = it->second[j].first;
            break;
          }
        }
        if (selected_driver < 0) selected_driver = it->second.back().first;
      }
    }
  }

  if (selected_driver < 0) {
    // Fallback: should not happen if GetMeanFreePath returned finite
    ldmx_log(error) << "No matching GENIE target found for material "
                    << material->GetName() << " — returning unchanged track";
    return G4VDiscreteProcess::PostStepDoIt(track, step);
  }

  // Generate the GENIE event
  genie::EventRecord* genie_event = nullptr;
  while (!genie_event) {
    genie_event = evg_drivers_[selected_driver].GenerateEvent(e_p4);
  }

  // Store HepMC3 event record in UserEventInformation
  auto* ev_info = static_cast<UserEventInformation*>(
      G4EventManager::GetEventManager()->GetUserInformation());
  if (ev_info) {
    auto hepmc3_genie = hep_mc3_converter_.ConvertToHepMC3(*genie_event);
    ldmx::HepMC3GenEvent hepmc3_ldmx_genie;
    hepmc3_genie->write_data(hepmc3_ldmx_genie);
    ev_info->addHepMC3GenEvent(hepmc3_ldmx_genie);

    // Propagate event weight
    ev_info->incWeight(genie_event->Weight());
  }

  // Count final-state particles
  int n_entries = genie_event->GetEntries();
  int n_secondaries = 0;
  for (int i = 0; i < n_entries; ++i) {
    auto* p = static_cast<genie::GHepParticle*>((*genie_event)[i]);
    if (p->Status() == 1) ++n_secondaries;
  }

  aParticleChange.SetNumberOfSecondaries(n_secondaries);

  // Interaction position
  G4ThreeVector position = step.GetPostStepPoint()->GetPosition();
  G4double global_time = step.GetPostStepPoint()->GetGlobalTime();

  // Add final-state particles as secondaries
  for (int i = 0; i < n_entries; ++i) {
    auto* p = static_cast<genie::GHepParticle*>((*genie_event)[i]);
    if (p->Status() != 1) continue;

    int pdg = p->Pdg();
    G4ParticleDefinition* particle_def = nullptr;

    // Handle nuclear fragments / ions
    if (pdg > 1000000000) {
      int ion_Z = (pdg / 10000) % 1000;
      int ion_A = (pdg / 10) % 1000;
      particle_def = G4IonTable::GetIonTable()->GetIon(ion_Z, ion_A, 0.);
    } else {
      particle_def = G4ParticleTable::GetParticleTable()->FindParticle(pdg);
    }

    if (!particle_def) {
      ldmx_log(warn) << "Could not find G4ParticleDefinition for PDG=" << pdg
                     << " — skipping this secondary";
      continue;
    }

    G4ThreeVector momentum(p->Px() * CLHEP::GeV, p->Py() * CLHEP::GeV,
                           p->Pz() * CLHEP::GeV);

    auto* dynamic_particle = new G4DynamicParticle(particle_def, momentum);
    auto* secondary_track =
        new G4Track(dynamic_particle, global_time, position);
    secondary_track->SetParentID(track.GetTrackID());

    aParticleChange.AddSecondary(secondary_track);
  }

  // Kill the primary electron
  aParticleChange.ProposeTrackStatus(fStopAndKill);

  delete genie_event;

  return G4VDiscreteProcess::PostStepDoIt(track, step);
}

}  // namespace simcore
