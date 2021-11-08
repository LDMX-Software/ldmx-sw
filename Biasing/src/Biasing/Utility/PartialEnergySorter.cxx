
#include "Biasing/Utility/PartialEnergySorter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"

namespace biasing {
namespace utility {

PartialEnergySorter::PartialEnergySorter(const std::string& name,
                                         framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");
}

void PartialEnergySorter::BeginOfEventAction(const G4Event*) {
  /** debug printout
  std::cout << "[ PartialEnergySorter ] : "
      << "Starting a new event."
      << std::endl;
   */
  below_threshold_ = false;
}

G4ClassificationOfNewTrack PartialEnergySorter::ClassifyNewTrack(
    const G4Track* aTrack,
    const G4ClassificationOfNewTrack& currentTrackClassification) {
  /** debug printout
  std::cout << "[ PartialEnergySorter ] : Classifying track "
      << aTrack->GetTrackID() << " with energy "
      << aTrack->GetKineticEnergy() << " MeV."
      << std::endl;
   */

  if (aTrack->GetKineticEnergy() > threshold_) {
    /** debug printout
    std::cout << "[ PartialEnergySorter ] : Classifying track "
        << aTrack->GetTrackID() << " with energy "
        << aTrack->GetKineticEnergy() << " MeV."
        << std::endl;
     */
    return fUrgent;
  }

  /*
   * Track has kinetic energy less than or equal to
   * the threshold, so we put it on the waiting stack
   * if there are still particles above threshold to be processed.
   */
  return below_threshold_ ? currentTrackClassification : fWaiting;
}

void PartialEnergySorter::stepping(const G4Step* step) {
  if (below_threshold_) return;

  auto pre_energy{step->GetPreStepPoint()->GetKineticEnergy()};
  auto post_energy{step->GetPostStepPoint()->GetKineticEnergy()};

  if (pre_energy >= threshold_ and post_energy <= threshold_) {
    /** debug printout
    std::cout << "[ PartialEnergySorter ] : Stepping track "
        << step->GetTrack()->GetTrackID() << " going from "
        << pre_energy  << " MeV to "
        << post_energy << " MeV."
        << std::endl;
     */
    step->GetTrack()->SetTrackStatus(fSuspend);
  }
}

}  // namespace utility
}  // namespace biasing

DECLARE_ACTION(biasing::utility, PartialEnergySorter)
