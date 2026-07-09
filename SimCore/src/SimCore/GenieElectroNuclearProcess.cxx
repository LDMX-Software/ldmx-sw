/**
 * @file GenieElectroNuclearProcess.cxx
 * @brief Implementation of the GENIE-based electronuclear G4VDiscreteProcess.
 */

#include "SimCore/GenieElectroNuclearProcess.h"

#include "SimCore/G4User/UserEventInformation.h"
#include "SimCore/G4User/VolumeChecks.h"

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
#include "G4Element.hh"
#include "G4EventManager.hh"
#include "G4IonTable.hh"
#include "G4Isotope.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "Randomize.hh"

// ROOT
#include <TLorentzVector.h>
#include <TParticle.h>

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <fstream>

namespace simcore {

const std::string GenieElectroNuclearProcess::PROCESS_NAME = "electronNuclear";

GenieElectroNuclearProcess::GenieElectroNuclearProcess(
    const framework::config::Parameters& params)
    : G4VDiscreteProcess(PROCESS_NAME, fElectromagnetic) {
  // Use a distinct EM subtype so we don't collide with other EM processes.
  // 64 is arbitrary but distinct from standard EM subtypes and the DarkBrem
  // subtype (63).
  SetProcessSubType(64);

  // Read configuration (targets/abundances are optional — empty means
  // auto-discover)
  targets_ = params.get<std::vector<int>>("targets", {});
  abundances_ = params.get<std::vector<double>>("abundances", {});
  discover_volume_ = params.get<std::string>("discover_volume", "");
  tune_ = params.get<std::string>("tune");
  spline_file_ = params.get<std::string>("spline_file");
  message_threshold_file_ = params.get<std::string>("message_threshold_file");
  only_one_per_event_ = params.get<bool>("only_one_per_event");

  // Initialize GENIE framework (tune, splines) — independent of targets
  initializeGENIE();

  if (!targets_.empty()) {
    // Manual mode: user-specified targets and abundances
    // Normalize abundances
    double abundance_sum = 0;
    for (auto a : abundances_) abundance_sum += a;
    if (abundance_sum > 0) {
      for (auto& a : abundances_) a /= abundance_sum;
    }

    setupDrivers();

    // Build the Z -> targets lookup map
    for (size_t i = 0; i < targets_.size(); ++i) {
      int z = (targets_[i] / 10000) % 1000;
      z_to_targets_[z].emplace_back(static_cast<int>(i), abundances_[i]);
    }

    ldmx_log(info) << "GenieElectroNuclearProcess configured with "
                   << targets_.size() << " manual target(s), tune=" << tune_;
  } else {
    // Auto-discovery mode: targets are discovered once from the configured
    // volume on the first GetMeanFreePath call (geometry exists by then).
    if (discover_volume_.empty()) {
      EXCEPTION_RAISE(
          "ConfigurationException",
          "GenieElectroNuclearProcess is in auto-discovery mode (no manual "
          "'targets') but 'discover_volume' is empty.  Set genie_nuclear."
          "discover_volume to the volume to discover targets from (e.g. "
          "\"target_region\"), or specify 'targets' and 'abundances' "
          "manually.");
    }
    auto_discover_ = true;
    ldmx_log(info) << "GenieElectroNuclearProcess configured for auto-discovery"
                   << " from volume '" << discover_volume_
                   << "', tune=" << tune_;
  }
}

GenieElectroNuclearProcess::~GenieElectroNuclearProcess() {
  ldmx_log(info) << "--- GENIE Process Summary ---";
  for (size_t i = 0; i < targets_.size(); ++i) {
    ldmx_log(info) << "  Target=" << targets_[i]
                   << "  Abundance=" << abundances_[i];
  }
  ldmx_log(info) << "--- GENIE Process Summary END ---";
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

  // Record which target nuclei actually have splines so we can skip the rest
  loadAvailableTargets();

  genie::GHepRecord::SetPrintLevel(0);
}

void GenieElectroNuclearProcess::loadAvailableTargets() {
  std::ifstream in(spline_file_);
  if (!in) {
    ldmx_log(warn) << "Could not open spline file '" << spline_file_
                   << "' to determine available targets; all discovered "
                   << "isotopes will be attempted.";
    return;
  }

  // Spline keys embed the target nucleus as a "tgt:<10-digit-code>" token.
  const std::string token = "tgt:";
  std::string line;
  while (std::getline(in, line)) {
    size_t pos = 0;
    while ((pos = line.find(token, pos)) != std::string::npos) {
      pos += token.size();
      size_t end = pos;
      while (end < line.size() &&
             std::isdigit(static_cast<unsigned char>(line[end]))) {
        ++end;
      }
      if (end > pos) {
        available_targets_.insert(std::stoi(line.substr(pos, end - pos)));
      }
      pos = end;
    }
  }

  ldmx_log(info) << "Found cross-section splines for "
                 << available_targets_.size() << " target nuclei in "
                 << spline_file_;
}

bool GenieElectroNuclearProcess::splineAvailable(int target_code) const {
  // Fail open: if we could not parse the spline file, don't filter anything.
  if (available_targets_.empty()) return true;
  return available_targets_.count(target_code) > 0;
}

void GenieElectroNuclearProcess::setupDrivers() {
  for (size_t i = 0; i < targets_.size(); ++i) {
    if (!splineAvailable(targets_[i])) {
      ldmx_log(error) << "No cross-section spline available for manually "
                      << "specified target " << targets_[i]
                      << " in spline file " << spline_file_;
      EXCEPTION_RAISE(
          "GenieSplineMissing",
          "No GENIE cross-section spline for target " +
              std::to_string(targets_[i]) +
              ". Add it to the spline file or remove it from 'targets'.");
    }
    auto driver = std::make_unique<genie::GEVGDriver>();
    genie::InitialState initial_state(targets_[i], 11);  // electron probe
    driver->SetEventGeneratorList(
        genie::RunOpt::Instance()->EventGeneratorList());
    driver->SetUnphysEventMask(*genie::RunOpt::Instance()->UnphysEventMask());
    driver->Configure(initial_state);
    driver->UseSplines();
    evg_drivers_.push_back(std::move(driver));
  }
}

void GenieElectroNuclearProcess::discoverIsotopesForElement(
    const G4Element* element) {
  int z = static_cast<int>(element->GetZ());

  // Already checked this element
  if (!discovered_z_.insert(z).second) return;

  size_t n_isotopes = element->GetNumberOfIsotopes();

  if (n_isotopes > 0) {
    // Element has explicit isotope data — use it
    const G4IsotopeVector* isotopes = element->GetIsotopeVector();
    const G4double* rel_ab = element->GetRelativeAbundanceVector();

    for (size_t j = 0; j < n_isotopes; ++j) {
      int iso_z = (*isotopes)[j]->GetZ();
      int a = (*isotopes)[j]->GetN();
      double abundance = rel_ab[j];
      if (abundance <= 0) continue;

      int target_code = 1000000000 + iso_z * 10000 + a * 10;

      if (!splineAvailable(target_code)) {
        ldmx_log(warn) << "No cross-section spline available for target "
                       << target_code << " (Z=" << iso_z << ", A=" << a
                       << ") from element " << element->GetName()
                       << " — skipping it (computing on the fly would be "
                       << "prohibitively slow).";
        continue;
      }

      int driver_idx = static_cast<int>(evg_drivers_.size());

      targets_.push_back(target_code);
      abundances_.push_back(abundance);

      auto driver = std::make_unique<genie::GEVGDriver>();
      genie::InitialState initial_state(target_code, 11);
      driver->SetEventGeneratorList(
          genie::RunOpt::Instance()->EventGeneratorList());
      driver->SetUnphysEventMask(*genie::RunOpt::Instance()->UnphysEventMask());
      driver->Configure(initial_state);
      driver->UseSplines();
      evg_drivers_.push_back(std::move(driver));

      z_to_targets_[z].emplace_back(driver_idx, abundance);

      ldmx_log(info) << "Auto-discovered target " << target_code << " (Z=" << z
                     << ", A=" << a << ", abundance=" << abundance << ")";
    }
  } else {
    // No explicit isotopes — use Z and rounded atomic mass as single target
    int a = static_cast<int>(std::round(element->GetAtomicMassAmu()));
    int target_code = 1000000000 + z * 10000 + a * 10;

    if (!splineAvailable(target_code)) {
      ldmx_log(warn) << "No cross-section spline available for target "
                     << target_code << " (Z=" << z << ", A=" << a
                     << ") from element " << element->GetName()
                     << " — skipping it (computing on the fly would be "
                     << "prohibitively slow).";
      return;
    }

    int driver_idx = static_cast<int>(evg_drivers_.size());

    targets_.push_back(target_code);
    abundances_.push_back(1.0);

    auto driver = std::make_unique<genie::GEVGDriver>();
    genie::InitialState initial_state(target_code, 11);
    driver->SetEventGeneratorList(
        genie::RunOpt::Instance()->EventGeneratorList());
    driver->SetUnphysEventMask(*genie::RunOpt::Instance()->UnphysEventMask());
    driver->Configure(initial_state);
    driver->UseSplines();
    evg_drivers_.push_back(std::move(driver));

    z_to_targets_[z].emplace_back(driver_idx, 1.0);

    ldmx_log(info) << "Auto-discovered target " << target_code << " (Z=" << z
                   << ", A=" << a << ") from element " << element->GetName();
  }
}

void GenieElectroNuclearProcess::discoverFromVolume() {
  namespace vc = simcore::g4user::volumechecks;

  // Select the same volume-matching test the ElectroNuclear bias operator
  // uses, so the discovered targets correspond to the biased volume.
  using Test = bool (*)(G4LogicalVolume*, const std::string&);
  Test include_volume_test = nullptr;
  if (discover_volume_ == "ecal") {
    include_volume_test = &vc::isInEcal;
  } else if (discover_volume_ == "old_ecal") {
    include_volume_test = &vc::isInEcalOld;
  } else if (discover_volume_ == "target") {
    include_volume_test = &vc::isInTargetOnly;
  } else if (discover_volume_ == "target_region") {
    include_volume_test = &vc::isInTargetRegion;
  } else if (discover_volume_ == "hcal") {
    include_volume_test = &vc::isInHcal;
  } else {
    include_volume_test = &vc::nameContains;
  }

  int n_volumes = 0;
  for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
    if (!include_volume_test(volume, discover_volume_)) continue;
    ++n_volumes;

    G4Material* mat = volume->GetMaterial();
    if (!mat) continue;
    const G4ElementVector* els = mat->GetElementVector();
    for (size_t i = 0; i < mat->GetNumberOfElements(); ++i) {
      // discoverIsotopesForElement de-duplicates by Z internally
      discoverIsotopesForElement((*els)[i]);
    }
  }

  if (evg_drivers_.empty()) {
    ldmx_log(warn) << "Auto-discovery found no target isotopes in volume(s) "
                   << "matching '" << discover_volume_ << "' (" << n_volumes
                   << " volume(s) matched). Electronuclear interactions will "
                   << "not fire — check the discover_volume setting.";
  } else {
    ldmx_log(info) << "Auto-discovered " << evg_drivers_.size()
                   << " target isotope(s) from " << n_volumes
                   << " volume(s) matching '" << discover_volume_ << "'";
  }
}

G4bool GenieElectroNuclearProcess::IsApplicable(const G4ParticleDefinition& p) {
  return &p == G4Electron::Definition();
}

G4double GenieElectroNuclearProcess::GetMeanFreePath(
    const G4Track& track, G4double /*prevStepSize*/,
    G4ForceCondition* /*condition*/) {
  if (!IsApplicable(*track.GetParticleDefinition())) return DBL_MAX;

  // One-shot auto-discovery: the geometry is guaranteed to be built by the
  // first call to GetMeanFreePath, so we look up the configured volume's
  // materials once and set up only the relevant GENIE drivers.
  if (auto_discover_) {
    discoverFromVolume();
    auto_discover_ = false;
  }

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
  const G4double* atom_density = material->GetVecNbOfAtomsPerVolume();
  size_t n_elements = material->GetNumberOfElements();

  G4double sigma = 0;
  partial_sum_sigma_.resize(n_elements);

  for (size_t i = 0; i < n_elements; ++i) {
    int z = static_cast<int>((*elements)[i]->GetZ());

    G4double element_sigma = 0;
    auto it = z_to_targets_.find(z);
    if (it != z_to_targets_.end()) {
      for (const auto& [driver_idx, abundance] : it->second) {
        // Query GENIE for this target's cross section
        double xsec_genie = evg_drivers_[driver_idx]->XSecSum(e_p4);
        // Convert from GENIE internal units to Geant4 units
        double xsec_g4 =
            (xsec_genie / genie::units::millibarn) * CLHEP::millibarn;
        element_sigma += abundance * xsec_g4;
      }
    }
    // else: no GENIE target for this Z, contributes 0

    sigma += atom_density[i] * element_sigma;
    partial_sum_sigma_[i] = sigma;
  }

  return sigma > DBL_MIN ? 1.0 / sigma : DBL_MAX;
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
    int z = static_cast<int>((*elements)[0]->GetZ());
    auto it = z_to_targets_.find(z);
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

    int z = static_cast<int>((*elements)[selected_element]->GetZ());
    auto it = z_to_targets_.find(z);
    if (it != z_to_targets_.end() && !it->second.empty()) {
      if (it->second.size() == 1) {
        selected_driver = it->second[0].first;
      } else {
        // Weighted random among isotopes for this Z
        // Weight by abundance * xsec
        std::vector<double> weights;
        double total_w = 0;
        for (const auto& [idx, ab] : it->second) {
          double xsec = evg_drivers_[idx]->XSecSum(e_p4);
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
    genie_event = evg_drivers_[selected_driver]->GenerateEvent(e_p4);
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
      int ion_z = (pdg / 10000) % 1000;
      int ion_a = (pdg / 10) % 1000;
      particle_def = G4IonTable::GetIonTable()->GetIon(ion_z, ion_a, 0.);
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
