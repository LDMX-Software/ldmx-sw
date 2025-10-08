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
  ecal_veto_name_ = ps.get<std::string>("ecal_veto_name");
  ecal_veto_pass_ = ps.get<std::string>("ecal_veto_pass");
  ecal_mip_name_ = ps.get<std::string>("ecal_mip_name");
  ecal_mip_pass_ = ps.get<std::string>("ecal_mip_pass");
  summed_det_max_ = ps.get<double>("summed_det_max");              // MeV
  summed_tight_iso_max_ = ps.get<double>("summed_tight_iso_max");  // MeV
  ecal_back_energy_max_ = ps.get<double>("ecal_back_energy_max");  // MeV
  n_readout_hits_max_ = ps.get<int>("n_readout_hits_max");
  shower_rms_max_ = ps.get<double>("shower_rms_max");
  shower_y_std_max_ = ps.get<double>("shower_y_std_max");
  shower_x_std_max_ = ps.get<double>("shower_x_std_max");
  max_cell_dep_max_ = ps.get<double>("max_cell_dep_max");  // MeV
  std_layer_hit_max_ = ps.get<int>("std_layer_hit_max");
  n_straight_tracks_max_ = ps.get<int>("n_straight_tracks_max");
  bdt_disc_min_ = ps.get<double>("bdt_disc_min");
  fiducial_level_ = ps.get<int>("fiducial_level");

  return;
}

void EcalPreselectionSkimmer::produce(framework::Event &event) {
  bool passed_preselection{false};
  bool fiducial_decision{true};
  const auto &ecal_veto{
      event.getObject<ldmx::EcalVetoResult>(ecal_veto_name_, ecal_veto_pass_)};
  const auto &mip_result{
      event.getObject<ldmx::EcalMipResult>(ecal_mip_name_, ecal_mip_pass_)};
  // Boolean to if we skim for fiducial / nonfiducial
  fiducial_decision = (fiducial_level_ == 0 ||
                       (fiducial_level_ == 1 && ecal_veto.getFiducial()) ||
                       (fiducial_level_ == 2 && !ecal_veto.getFiducial()));

  // Boolean to check if we pass preselection
  passed_preselection =
      (ecal_veto.getSummedDet() < summed_det_max_) &&
      (ecal_veto.getSummedTightIso() < summed_tight_iso_max_) &&
      (ecal_veto.getEcalBackEnergy() < ecal_back_energy_max_) &&
      (ecal_veto.getNReadoutHits() < n_readout_hits_max_) &&
      (ecal_veto.getShowerRMS() < shower_rms_max_) &&
      (ecal_veto.getYStd() < shower_y_std_max_) &&
      (ecal_veto.getXStd() < shower_x_std_max_) &&
      (ecal_veto.getMaxCellDep() < max_cell_dep_max_) &&
      (ecal_veto.getStdLayerHit() < std_layer_hit_max_) &&
      (mip_result.getNStraightTracks() < n_straight_tracks_max_) &&
      (ecal_veto.getDisc() > bdt_disc_min_) && fiducial_decision;

  // Tell the skimmer to keep or drop the event based on whether preselection
  // passed
  if (passed_preselection) {
    ldmx_log(debug) << "This event passed preselection!";
    setStorageHint(framework::HINT_SHOULD_KEEP);
  } else {
    setStorageHint(framework::HINT_SHOULD_DROP);
  }
  // Add the boolean to the event
  event.add("EcalPreselectionDecision", passed_preselection);
}
}  // namespace recon

DECLARE_PRODUCER(recon::EcalPreselectionSkimmer);
