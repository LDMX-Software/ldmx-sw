/**
 * @file EcalPreselectionSkimmer.cxx
 * @brief Processor used to pre-select events for the ECAL studies
 * @author Tamas Almos Vami (UCSB)
 */

#include "Recon/Skims/EcalPreselectionSkimmer.h"

namespace recon {

EcalPreselectionSkimmer::EcalPreselectionSkimmer(const std::string &name,
                                                 framework::Process &process)
    : framework::Producer(name, process) {}

void EcalPreselectionSkimmer::configure(framework::config::Parameters &ps) {
  ecal_veto_name_ = ps.getParameter<std::string>("ecal_veto_name");
  ecal_veto_pass_ = ps.getParameter<std::string>("ecal_veto_pass");
  summed_det_max_ = ps.getParameter<double>("summed_det_max");  // MeV
  summed_tight_iso_max_ =
      ps.getParameter<double>("summed_tight_iso_max");  // MeV
  ecal_back_energy_max_ =
      ps.getParameter<double>("ecal_back_energy_max");  // MeV
  n_readout_hits_max_ = ps.getParameter<int>("n_readout_hits_max");
  shower_rms_max_ = ps.getParameter<int>("shower_rms_max");
  shower_y_std_max_ = ps.getParameter<int>("shower_y_std_max");
  shower_x_std_max_ = ps.getParameter<int>("shower_x_std_max");
  max_cell_dep_max_ = ps.getParameter<double>("max_cell_dep_max");  // MeV
  std_layer_hit_max_ = ps.getParameter<int>("std_layer_hit_max");
  n_straight_tracks_max_ = ps.getParameter<int>("n_straight_tracks_max");

  return;
}

void EcalPreselectionSkimmer::produce(framework::Event &event) {
  bool passedPreselection{false};
  const auto &ecalVeto{
      event.getObject<ldmx::EcalVetoResult>(ecal_veto_name_, ecal_veto_pass_)};

  // Boolean to check if we pass preselection
  passedPreselection = (ecalVeto.getSummedDet() < summed_det_max_) &&
                       (ecalVeto.getSummedTightIso() < summed_tight_iso_max_) &&
                       (ecalVeto.getEcalBackEnergy() < ecal_back_energy_max_) &&
                       (ecalVeto.getNReadoutHits() < n_readout_hits_max_) &&
                       (ecalVeto.getShowerRMS() < shower_rms_max_) &&
                       (ecalVeto.getYStd() < shower_y_std_max_) &&
                       (ecalVeto.getXStd() < shower_x_std_max_) &&
                       (ecalVeto.getMaxCellDep() < max_cell_dep_max_) &&
                       (ecalVeto.getStdLayerHit() < std_layer_hit_max_) &&
                       (ecalVeto.getNStraightTracks() < n_straight_tracks_max_);

  // Tell the skimmer to keep or drop the event based on whether preselection
  // passed
  if (passedPreselection) {
    ldmx_log(debug) << "This event passed preselection!";
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }
  // Add the boolean to the event
  event.add("EcalPreselectionDecision", passedPreselection);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, EcalPreselectionSkimmer);
