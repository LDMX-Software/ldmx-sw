
#include "Biasing/NonfiducialFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackInformation.h"

#include <cmath>

namespace biasing {

NonfiducialFilter::NonfiducialFilter(const std::string& name,
                                   framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  recoilAngleThreshold_ =
      parameters.getParameter<double>("recoil_angle_threshold");
  bremEnergyThreshold_ =
      parameters.getParameter<double>("brem_min_energy_threshold");
  killRecoil_ = parameters.getParameter<bool>("kill_recoil_track");
}

NonfiducialFilter::~NonfiducialFilter() {}

G4ClassificationOfNewTrack NonfiducialFilter::ClassifyNewTrack(
    const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass) {
  // get the PDGID of the track.
  G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

  // Get the particle type.
  G4String particleName = track->GetParticleDefinition()->GetParticleName();

  // Use current classification by default so values from other plugins are not
  // overridden.
  G4ClassificationOfNewTrack classification = currentTrackClass;

  if (track->GetTrackID() == 1 && pdgID == 11) {
    return fWaiting;
  }

  return classification;
}

void NonfiducialFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Only process the primary electron track
  if (track->GetParentID() != 0) return;

  // Get the PDG ID of the track and make sure it's an electron. If
  // another particle type is found, thrown an exception.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11)
    return;

  // Get the region the particle is currently in.  Continue processing
     // the particle only if it's in the target region.
  if (auto region{
          track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
      region.compareTo("target") != 0)
    return;
                           
  // Check if the electron will be exiting the target
  if (auto volume{track->GetNextVolume()->GetName()};
      volume.compareTo("recoil_PV") == 0) {
    // If the recoil electron misses the ECal Scoring Plane
    double xPos{track->GetPosition().getX()}; // X position
    double yPos{track->GetPosition().getY()}; // Y position
    double zPos{track->GetPosition().getZ()}; // Z position
    double xMom{track->GetMomentum().getX()}; // X momentum
    double yMom{track->GetMomentum().getY()}; // Y momentum
    double zMom{track->GetMomentum().getZ()}; // Z momentum
    
    double xProjection(double x, double y, double z, double xmom, double ymom, double zmom) {
        double x_final;
        double EcalSP = 240.5015;
        if (xmom == 0) {
          x_final = x + (EcalSP - z)/99999;
        } else {
          x_final = x + xmom/zmom*(EcalSP - zmom);
        } 
        return x_final;

    double yProjection(double x, double y, double z, double xmom, double ymom, double zmom) {
        double y_final;
        double EcalSP = 240.5015;
        if (ymom == 0) {
          y_final = y + (EcalSP - z)/99999;
        } else {
          y_final = y + ymom/zmom*(EcalSP - zmom);
        }
        return y_final;
    double xFinal = xProjection(xPos,yPos,zPos,xMom,yMom,zMom)
    double yFinal = yProjection(xPos,yPos,zPos,xMom,yMom,zMom)
    if (xFinal < 246.6734 && xFinal > -246.6734 && yFinal < 256.5005 && yFinal > -256.5005) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    if (!hasBremCandidate) {
      track->SetTrackStatus(fKillTrackAndSecondaries);
      G4RunManager::GetRunManager()->AbortEvent();
      return;
    }

    // Check if the recoil electron should be killed.  If not, postpone
    // its processing until the brem gamma has been processed.
    if (killRecoil_)
      track->SetTrackStatus(fStopAndKill);
    else
      track->SetTrackStatus(fSuspend);

  } else if (step->GetPostStepPoint()->GetKineticEnergy() == 0) {
    track->SetTrackStatus(fKillTrackAndSecondaries);
    G4RunManager::GetRunManager()->AbortEvent();
    return;
  }
}

void NonfiducialFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing, NonfiducialFilter)
