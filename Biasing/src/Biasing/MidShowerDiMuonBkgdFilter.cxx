#include "Biasing/MidShowerDiMuonBkgdFilter.h"

#include "G4EventManager.hh"
#include "G4Gamma.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "SimCore/UserTrackInformation.h"

namespace biasing {

MidShowerDiMuonBkgdFilter::MidShowerDiMuonBkgdFilter(
    const std::string& name, framework::config::Parameters& parameters)
    : simcore::UserAction(name, parameters) {
  threshold_ = parameters.getParameter<double>("threshold");
  /* debug printout
  std::cout
      << "[ MidShowerDiMuonBkgdFilter ]: "
      << "created instance " << name
      << "with threshold " << threshold_ << " MeV"
      << std::endl;
   */
}

void MidShowerDiMuonBkgdFilter::BeginOfEventAction(const G4Event*) {
  /* debug printout
  std::cout
      << "[ MidShowerDiMuonBkgdFilter ]: "
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") "
      << "starting new simulation event." << std::endl;
   */
  total_process_energy_ = 0.;
}

void MidShowerDiMuonBkgdFilter::stepping(const G4Step* step) {
  // skips steps that aren't done by photons (required for mu conv)
  if (step->GetTrack()->GetParticleDefinition() != G4Gamma::Gamma()) return;
  // skip steps that are outside the calorimeter region
  if (isOutsideCalorimeterRegion(step)) return;
  // check photon secondaries for muons
  const G4TrackVector* secondaries{step->GetSecondary()};
  if (secondaries == nullptr or secondaries->size() == 0) return;
  // have left if secondaries is empty
  bool found_muon{false};
  for (const G4Track* secondary : *secondaries) {
    if (abs(secondary->GetParticleDefinition()->GetPDGEncoding()) == 13) {
      found_muon = true;
      save(secondary);
    }
  }
  if (not found_muon) return;
  // there are interesting secondaries in this step
  double pre_energy = step->GetPreStepPoint()->GetTotalEnergy();
  double post_energy = step->GetPostStepPoint()->GetTotalEnergy();
  total_process_energy_ += pre_energy - post_energy;
  const G4Track* track = step->GetTrack();
  /* debug printout
  std::cout << "[ MidShowerDiMuonBkgdFilter ]: "
            << "("
            << G4EventManager::GetEventManager()
                   ->GetConstCurrentEvent()
                   ->GetEventID()
            << ") "
            << "Track " << track->GetParentID() << " created "
            << track->GetTrackID() << " which went from " << pre_energy
            << " MeV to " << post_energy << " MeV generating muons."
            << std::endl;
   */
  // make sure this track is saved
  save(track);
}

void MidShowerDiMuonBkgdFilter::NewStage() {
  /* debug printout
  std::cout
      << "[ MidShowerDiMuonBkgdFilter ]: "
      << "("
      << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
      << ") " << total_process_energy_ << " MeV was muonic." << std::endl;
   */
  if (total_process_energy_ < threshold_)
    AbortEvent("Not enough energy went to the input process.");
  return;
}

bool MidShowerDiMuonBkgdFilter::isOutsideCalorimeterRegion(
    const G4Step* step) const {
  // the pointers in this chain are assumed to be always valid
  auto reg{step->GetTrack()->GetVolume()->GetLogicalVolume()->GetRegion()};
  if (reg) return (reg->GetName() != "CalorimeterRegion");
  // region is nullptr ==> no region defined for current volume
  //  ==> outside CalorimeterRegion
  return true;
}

void MidShowerDiMuonBkgdFilter::save(const G4Track* track) const {
  auto track_info{simcore::UserTrackInformation::get(track)};
  track_info->setSaveFlag(true);
  return;
}

void MidShowerDiMuonBkgdFilter::AbortEvent(const std::string& reason) const {
  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
    std::cout << "[ MidShowerDiMuonBkgdFilter ]: "
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

DECLARE_ACTION(biasing, MidShowerDiMuonBkgdFilter)
