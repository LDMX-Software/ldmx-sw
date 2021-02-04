#include "Biasing/PhotoNuclearProductsFilter.h"

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

PhotoNuclearProductsFilter::PhotoNuclearProductsFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  productsPdgID_ = parameters.getParameter<std::vector<int> >("pdg_ids");
}

PhotoNuclearProductsFilter::~PhotoNuclearProductsFilter() {}

void PhotoNuclearProductsFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Get the track info and check if this track has been tagged as the
  // photon that underwent a photo-nuclear reaction. Only those tracks
  // tagged as PN photos will be processed. The track is currently only
  // tagged by the UserAction ECalProcessFilter which needs to be run
  // before this UserAction.
  auto trackInfo{
      static_cast<simcore::UserTrackInformation*>(track->GetUserInformation())};
  if ((trackInfo != nullptr) && !trackInfo->isPNGamma()) return;

  // Get the PN photon daughters.
  auto secondaries{step->GetSecondary()};

  // Loop through all of the secondaries and check for the product of
  // interest.  This is done by getting the PDG ID of a daughter and
  // checking that it's in the vector of PDG IDs passed to this
  // UserAction.
  bool productFound{false};
  for (const auto& secondary : *secondaries) {
    // Get the PDG ID of the track
    auto pdgID{std::abs(secondary->GetParticleDefinition()->GetPDGEncoding())};

    // Check if the PDG ID is in the list of products of interest
    if (std::find(productsPdgID_.begin(), productsPdgID_.end(), pdgID) !=
        productsPdgID_.end()) {
      productFound = true;
      break;
    }
  }

  // If the product of interest was not found, kill the track and abort
  // the event.
  if (!productFound) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }

  // Once the PN gamma has been procesed, untag it so its not reprocessed
  // again.
  trackInfo->tagPNGamma(false);
}
}  // namespace biasing

DECLARE_ACTION(biasing, PhotoNuclearProductsFilter)
