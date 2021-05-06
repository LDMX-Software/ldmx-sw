/*
 * @file EcalDarkBremFilter.cxx
 * @class EcalDarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out
 *        events that don't result in a dark brem within a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/EcalDarkBremFilter.h"

#include "G4LogicalVolumeStore.hh"      //for the store
#include "SimCore/DarkBrem/G4APrime.h"  //checking if particles match A'
#include "SimCore/DarkBrem/G4eDarkBremsstrahlung.h"  //checking for dark brem secondaries
#include "SimCore/UserTrackInformation.h"            //make sure A' is saved

namespace biasing {

EcalDarkBremFilter::EcalDarkBremFilter(const std::string& name,
                                       framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");

  /*
   * We look for the logical volumes that match the following pattern:
   *  - 'volume' is in the name AND
   *  - 'Si' OR 'W' OR 'CFMix' OR 'PCB' are in the name
   */
  for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
    G4String volumeName = volume->GetName();
    // looking for ecal volumes
    if (volumeName.contains("volume") and
        (volumeName.contains("Si") or volumeName.contains("W") or
         volumeName.contains("CFMix") or volumeName.contains("PCB") or
         volumeName.contains("Al"))) {
      volumes_.push_back(volume);
    }
  }

  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 0) {
    std::cout << "[ EcalDarkBremFilter ]: "
              << "Looking for A' in: ";
    for (auto const& volume : volumes_) std::cout << volume->GetName() << ", ";
    std::cout << std::endl;
  }
}

void EcalDarkBremFilter::BeginOfEventAction(const G4Event*) {
  /* Debug message
  std::cout << "[ EcalDarkBremFilter ]: "
      << "(" <<
  G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << ")
  "
      << "Beginning event."
      << std::endl;
  */
  foundAp_ = false;
  return;
}

G4ClassificationOfNewTrack EcalDarkBremFilter::ClassifyNewTrack(
    const G4Track* aTrack, const G4ClassificationOfNewTrack& cl) {
  if (aTrack->GetParticleDefinition() == simcore::darkbrem::G4APrime::APrime()) {
    // there is an A'! Yay!
    /* Debug message
    std::cout << "[ EcalDarkBremFilter ]: "
              << "Found A', still need to check if it originated in requested
    volume."
              << std::endl;
    */
    if (not foundAp_ and aTrack->GetTotalEnergy() > threshold_) {
      // The A' is the first one created in this event and is above the energy
      // threshold
      foundAp_ = true;
    } else if (foundAp_) {
      AbortEvent("Found more than one A' during filtering.");
    } else {
      AbortEvent("A' was not produced above the required threshold.");
    }
  }

  return cl;
}

void EcalDarkBremFilter::NewStage() {
  if (not foundAp_) AbortEvent("A' wasn't produced.");

  return;
}

void EcalDarkBremFilter::PostUserTrackingAction(const G4Track* track) {
  /* Check that generational stacking is working
  std::cout << "[ EcalDarkBremFilter ]:"
      << track->GetTrackID() << " " <<
  track->GetParticleDefinition()->GetPDGEncoding()
      << std::endl;
  */

  const G4VProcess* creator = track->GetCreatorProcess();
  if (creator and creator->GetProcessName().contains(
                      simcore::darkbrem::G4eDarkBremsstrahlung::PROCESS_NAME)) {
    // make sure all secondaries of dark brem process are saved
    simcore::UserTrackInformation* userInfo = simcore::UserTrackInformation::get(track);
    // make sure A' is persisted into output file
    userInfo->setSaveFlag(true);
    if (track->GetParticleDefinition() == simcore::darkbrem::G4APrime::APrime()) {
      // check if A' was made in the desired volume and has the minimum energy
      if (not inDesiredVolume(track)) {
        AbortEvent("A' wasn't produced inside of the requested volume.");
      }  // A' was made in desired volume and has the minimum energy
    }    // track was A'
  }      // track created by dark brem process

  return;
}

bool EcalDarkBremFilter::inDesiredVolume(const G4Track* track) const {
  /**
   * Comparing the pointers to logical volumes isn't very robust.
   * TODO find a better way to do this
   */

  auto inVol = track->GetLogicalVolumeAtVertex();
  for (auto const& volume : volumes_) {
    if (inVol == volume) return true;
  }

  return false;
}

void EcalDarkBremFilter::AbortEvent(const std::string& reason) const {
  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
    std::cout << "[ EcalDarkBremFilter ]: "
              << "("
              << G4EventManager::GetEventManager()
                     ->GetConstCurrentEvent()
                     ->GetEventID()
              << ") " << reason << " Aborting event." << std::endl;
  }
  G4RunManager::GetRunManager()->AbortEvent();
  return;
}
}  // namespace biasing

DECLARE_ACTION(biasing, EcalDarkBremFilter)
