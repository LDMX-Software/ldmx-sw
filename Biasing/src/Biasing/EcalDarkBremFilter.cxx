/*
 * @file EcalDarkBremFilter.cxx
 * @class EcalDarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out
 *        events that don't result in a dark brem within a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/EcalDarkBremFilter.h"

namespace biasing {

EcalDarkBremFilter::EcalDarkBremFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  threshold_ = parameters.get<double>("threshold");

  /*
   * We look for the logical volumes that match the following pattern:
   *  - 'volume' is in the name AND
   *  - 'Si' OR 'W' OR 'CFMix' OR 'PCB' are in the name
   */
  for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
    G4String volume_name = volume->GetName();
    // looking for ecal volumes
    if (volume_name.contains("volume") and
        (volume_name.contains("Si") or volume_name.contains("W") or
         volume_name.contains("CFMix") or volume_name.contains("PCB") or
         volume_name.contains("Al"))) {
      volumes_.push_back(volume);
    }
  }

  ldmx_log(trace) << "Looking for A' in: ";
  for (auto const& volume : volumes_) {
    ldmx_log(trace) << "\t" << volume->GetName() << ", ";
  }
}

void EcalDarkBremFilter::BeginOfEventAction(const G4Event*) {
  found_ap_ = false;
  return;
}

G4ClassificationOfNewTrack EcalDarkBremFilter::ClassifyNewTrack(
    const G4Track* aTrack, const G4ClassificationOfNewTrack& cl) {
  if (aTrack->GetParticleDefinition() == G4APrime::APrime()) {
    // there is an A'! Yay!
    ldmx_log(trace) << "Found A', still need to check if it originated in "
                       "requested volume.";

    if (not found_ap_ and aTrack->GetTotalEnergy() > threshold_) {
      // The A' is the first one created in this event and is above the energy
      // threshold
      found_ap_ = true;
    } else if (found_ap_) {
      AbortEvent("Found more than one A' during filtering.");
    } else {
      AbortEvent("A' was not produced above the required threshold.");
    }
  }

  return cl;
}

void EcalDarkBremFilter::NewStage() {
  if (not found_ap_) AbortEvent("A' wasn't produced.");

  return;
}

void EcalDarkBremFilter::PostUserTrackingAction(const G4Track* track) {
  // Check that generational stacking is working
  ldmx_log(trace) << track->GetTrackID() << " "
                  << track->GetParticleDefinition()->GetPDGEncoding();

  const G4VProcess* creator = track->GetCreatorProcess();
  if (creator and
      creator->GetProcessName().contains(G4DarkBremsstrahlung::PROCESS_NAME)) {
    // make sure all secondaries of dark brem process are saved
    simcore::UserTrackInformation* user_info =
        simcore::UserTrackInformation::get(track);
    // make sure A' is persisted into output file
    user_info->setSaveFlag(true);
    if (track->GetParticleDefinition() == G4APrime::APrime()) {
      // check if A' was made in the desired volume and has the minimum energy
      if (not inDesiredVolume(track)) {
        AbortEvent("A' wasn't produced inside of the requested volume.");
      }  // A' was made in desired volume and has the minimum energy
    }  // track was A'
  }  // track created by dark brem process

  return;
}

bool EcalDarkBremFilter::inDesiredVolume(const G4Track* track) const {
  /**
   * Comparing the pointers to logical volumes isn't very robust.
   * TODO find a better way to do this
   */

  auto in_vol = track->GetLogicalVolumeAtVertex();
  for (auto const& volume : volumes_) {
    if (in_vol == volume) return true;
  }

  return false;
}

void EcalDarkBremFilter::AbortEvent(const std::string& reason) const {
  ldmx_log(trace)
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") " << reason << " Aborting event.";

  G4RunManager::GetRunManager()->AbortEvent();
  return;
}
}  // namespace biasing

DECLARE_ACTION(biasing::EcalDarkBremFilter)
