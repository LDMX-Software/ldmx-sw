/*
 * @file EcalDarkBremFilter.cxx
 * @class EcalDarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out
 *        events that don't result in a dark brem within a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/EcalDarkBremFilter.h"

#include "SimCore/DarkBrem/G4APrime.h"     //checking if particles match A'
#include "SimCore/UserEventInformation.h"  //set the weight for the event
#include "SimCore/UserTrackInformation.h"  //make sure A' is saved

#include "G4LogicalVolumeStore.hh"  //for the store

namespace ldmx {

EcalDarkBremFilter::EcalDarkBremFilter(const std::string& name,
                                       Parameters& parameters)
    : UserAction(name, parameters) {
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
         volumeName.contains("CFMix") or volumeName.contains("PCB"))) {
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
  if (aTrack->GetParticleDefinition() == darkbrem::G4APrime::APrime()) {
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

  if (track->GetParticleDefinition() == darkbrem::G4APrime::APrime()) {
    // check if A' was made in the desired volume and has the minimum energy
    auto event{G4EventManager::GetEventManager()};
    if (not inDesiredVolume(track)) {
      AbortEvent("A' wasn't produced inside of the requested volume.");
    } else {
      // add generation to UserTrackInformation
      UserTrackInformation* userInfo =
          dynamic_cast<UserTrackInformation*>(track->GetUserInformation());

      // make sure A' is persisted into output file
      userInfo->setSaveFlag(true);

      if (!event->GetUserInformation()) {
        event->SetUserInformation(new UserEventInformation);
      }
      static_cast<UserEventInformation*>(event->GetUserInformation())
          ->setWeight(track->GetWeight());
      /* debug printout to check weighting of events
      std::cout << "[ EcalDarkBremFilter ]: "
          << "(" << event->GetConstCurrentEvent()->GetEventID()
          << ") weighted "
          << std::setprecision(std::numeric_limits<double>::digits10 + 1)
          << track->GetWeight()
          << std::endl;
       */
    }  // A' was made in desired volume and has the minimum energy
  }    // track is A'

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
}  // namespace ldmx

DECLARE_ACTION(ldmx, EcalDarkBremFilter)
