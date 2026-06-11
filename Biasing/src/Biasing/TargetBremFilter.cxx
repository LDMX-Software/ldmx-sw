/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/TargetBremFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Electron.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/G4User/PtrRetrieval.h"
#include "SimCore/G4User/UserEventInformation.h"
#include "SimCore/G4User/UserTrackInformation.h"

/*~~~~~~~~~~~~~*/
/*  C++ StdLib */
/*~~~~~~~~~~~~~*/
#include <cmath>

namespace biasing {

TargetBremFilter::TargetBremFilter(const std::string& name,
                                   framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoil_max_p_threshold_ = parameters.get<double>("recoil_max_p_threshold");
  brem_energy_threshold_ = parameters.get<double>("brem_min_energy_threshold");
  brem_theta_min_ = parameters.get<double>("brem_theta_min");
  brem_theta_max_ = parameters.get<double>("brem_theta_max");
  dral_min_ = parameters.get<double>("dral_min");
  dral_max_ = parameters.get<double>("dral_max");
  kill_recoil_ = parameters.get<bool>("kill_recoil_track");
}

TargetBremFilter::~TargetBremFilter() {}

G4ClassificationOfNewTrack TargetBremFilter::ClassifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  // get the PDGID of the track.
  G4int pdg_id = track->GetParticleDefinition()->GetPDGEncoding();

  // Get the particle type.
  G4String particle_name = track->GetParticleDefinition()->GetParticleName();

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  if (track->GetTrackID() == 1 && pdg_id == 11) {
    return fWaiting;
  }

  return classification;
}

void TargetBremFilter::stepping(const G4Step* step) {
      int n_photons = 0;
      int n_pass_theta = 0;
      int n_pass_dral = 0;
      double last_dral = -1;
      double last_theta = -1;
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Only process the primary electron track
  if (track->GetParentID() != 0) return;

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception.
  if (auto pdg_id{track->GetParticleDefinition()->GetPDGEncoding()};
      pdg_id != 11)
    return;

  // Get the region the particle is currently in.  Continue processing
  // the particle only if it's in the target region.
  static auto target_region =
      simcore::g4user::ptrretrieval::getRegion("target");
  if (!target_region) {
    ldmx_log(warn) << "Region 'target' not found in Geant4 region store";
  }
  auto phy_vol{track->GetVolume()};
  auto log_vol{phy_vol ? phy_vol->GetLogicalVolume() : nullptr};
  auto track_region{log_vol ? log_vol->GetRegion() : nullptr};
  if (track_region != target_region) return;

  /*
  std::cout << "[TargetBremFilter] : Stepping primary electron in 'target'
  region into "
    << track->GetNextVolume()->GetName()
    << std::endl;
   */

  /**
   * Check if the electron will be exiting the target
   *
   * The 'recoil_PV' volume name is automatically constructed by Geant4's
   * GDML parser and was found by inspecting the geometry using a
   * visualization. This Physical Volume (PV) is associated with the
   * recoil parent volume and so it will break if the recoil parent volume
   * changes its name.
   *
   * We also check if the next volume is World_PV because in some geometries
   * (e.g. v14), there is a air-gap between the target region and the recoil.
   */
  auto recoil_physical_volume =
      simcore::g4user::ptrretrieval::getPhysicalVolume("recoil_PV");
  auto world_physical_volume =
      simcore::g4user::ptrretrieval::getPhysicalVolume("World_PV");
  if (!recoil_physical_volume) {
    ldmx_log(warn) << "Volume 'recoil_PV' not found in Geant4 volume store";
  }
  if (!world_physical_volume) {
    ldmx_log(warn) << "Volume 'World_PV' not found in Geant4 volume store";
  }
  auto track_volume = track->GetNextVolume();
  if (track_volume == recoil_physical_volume or
      track_volume == world_physical_volume) {
    // If the recoil electron
    if (track->GetMomentum().mag() >= recoil_max_p_threshold_) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    // Get the electron secondries
    bool has_brem_candidate = false;
    if (auto secondaries = step->GetSecondary(); secondaries->size() == 0) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    } else {


      for (auto& secondary_track : *secondaries) {
        auto electron = G4Electron::Definition();
        auto ebrem_process =
            simcore::g4user::ptrretrieval::getProcess(electron, "eBrem");
        if (!ebrem_process) {
          ldmx_log(warn) << "Process 'eBrem' not found in Geant4 process store";
        }

        if (ebrem_process && secondary_track->GetKineticEnergy() > brem_energy_threshold_){

	  //Check if secondary is photon
	  auto secondary_pdg_id = secondary_track->GetParticleDefinition()->GetPDGEncoding();
          if (secondary_pdg_id != 22) continue;
	  n_photons++;

          // Brem angle
          auto momentum = secondary_track->GetMomentum();
          double theta = std::atan2(std::sqrt(momentum.x() * momentum.x() +
                                              momentum.y() * momentum.y()),
                                    momentum.z());
          bool pass_brem_theta =
              theta >= brem_theta_min_ && theta <= brem_theta_max_;

          // Maximum and Minimum angle between outgoing Brem photons and
          // electrons
          auto gamma_mom = secondary_track->GetMomentum();
          auto electron_mom = track->GetMomentum();
          double gamma_eta = gamma_mom.eta();
          double gamma_phi = gamma_mom.phi();
          double electron_eta = electron_mom.eta();
          double electron_phi = electron_mom.phi();

          double dphi = std::atan2(std::sin(electron_phi - gamma_phi),
                                   std::cos(electron_phi - gamma_phi));
          double dral = std::sqrt((electron_eta - gamma_eta) *
                                      (electron_eta - gamma_eta) +
                                  dphi * dphi);
          bool pass_dral = dral >= dral_min_ && dral <= dral_max_;

	  if (pass_brem_theta) n_pass_theta++;
	  if (pass_dral) n_pass_dral++;

	  last_theta = theta;
	  last_dral = dral;

          if (pass_brem_theta && pass_dral) {
            auto track_info{
                simcore::UserTrackInformation::get(secondary_track)};
            track_info->tagBremCandidate();

            getEventInfo()->incBremCandidateCount();

            has_brem_candidate = true;
          }
        }
      }
    }

    if (!has_brem_candidate) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      std::cout << "not a candidate, returning" << std::endl;
      return;
    }

    //std::cout << "[TargetBremFilter] : Found brem candidate" << std::endl;
    ldmx_log(trace) << "[TargetBremFilter] : Found brem candidate";
    ldmx_log(trace) << "Passed theta = " << last_theta;
    ldmx_log(trace) << "Passed dral = " << last_dral;

    // Check if the recoil electron should be killed.  If not, postpone
    // its processing until the brem gamma has been processed.
    if (kill_recoil_)
      track->SetTrackStatus(fStopAndKill);
    else
      track->SetTrackStatus(fSuspend);

  } else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }
}

void TargetBremFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing::TargetBremFilter)
