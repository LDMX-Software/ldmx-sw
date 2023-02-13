
#include "Biasing/NothingHardFilter.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <cmath>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"

namespace biasing {

NothingHardFilter::NothingHardFilter(const std::string& name,
                                     framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters),
      nothingHardThreshold_{
          parameters.getParameter<double>("nothing_hard_threshold")} {}

NothingHardFilter::~NothingHardFilter() {}

void NothingHardFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  // std::cout << "Doing NothingHardFilter..." << std::endl;
  auto track{step->GetTrack()};

  // Get the track info and check if this track has been tagged as the
  // photon that underwent a photo-nuclear reaction. Only those tracks
  // tagged as PN photos will be processed. The track is currently only
  // tagged by the UserAction ECalProcessFilter which needs to be run
  // before this UserAction.
  auto trackInfo{simcore::UserTrackInformation::get(track)};
  if ((trackInfo != nullptr) && !trackInfo->isPNGamma()) return;
  // std::cout << "Track is PN Gamma!" << std::endl;
  // Get the PN photon daughters.
  auto secondaries{step->GetSecondary()};

  // Loop through all of the secondaries and check for the product of
  // interest.  This is done by getting the PDG ID of a daughter and
  // checking that it's in the vector of PDG IDs passed to this
  // UserAction.
  double totalEnergy{0.};
  static int NHCounter{0};
  static int NHFailCounter{0};
  for (const auto& secondary : *secondaries) {
    // Get the PDG ID of the track
    auto pdgID{std::abs(secondary->GetParticleDefinition()->GetPDGEncoding())};
    auto energy{secondary->GetKineticEnergy() / CLHEP::MeV};
    if (pdgID < 10000) {
      totalEnergy += energy;
    }
    if (energy >= nothingHardThreshold_) {
      ++NHFailCounter;
      // std::cout << "Killing event due to " << pdgID << " with energy " <<
      // energy
      //           << " MeV which is more than the threshold of "
      //           << nothingHardThreshold_ << "\nfail count " << NHFailCounter
      //           << " and success counter " << NHCounter << std::endl;
      track->SetTrackStatus(fKillTrackAndSecondaries);
      trackInfo->tagPNGamma(false);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }
    // Check if the PDG ID is in the list of products of interest
    // if (std::find(productsPdgID_.begin(), productsPdgID_.end(), pdgID) !=
    //     productsPdgID_.end()) {
    //   productFound = true;
    //   break;
    // }
  }
  std::cout << "Found nothing hard event!" << std::endl;
  ++NHCounter;
  // for (const auto& secondary : *secondaries) {
  //   std::cout << "Particle: "
  //             << secondary->GetParticleDefinition()->GetPDGEncoding()
  //             << " with energy " << secondary->GetKineticEnergy() /
  //             CLHEP::MeV
  //             << " MeV" << std::endl;
  // }
  std::cout << "Total energy: " << totalEnergy / CLHEP::MeV << " MeV"
            << std::endl;
  // // If the product of interest was not found, kill the track and abort
  // // the event.
  // if (!productFound) {
  // }

  // Once the PN gamma has been procesed, untag it so its not reprocessed
  // again.
  trackInfo->tagPNGamma(false);
}
}  // namespace biasing

DECLARE_ACTION(biasing, NothingHardFilter)
