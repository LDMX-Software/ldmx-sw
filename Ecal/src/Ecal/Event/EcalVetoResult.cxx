
#include "Ecal/Event/EcalVetoResult.h"

ClassImp(ldmx::EcalVetoResult);

namespace ldmx {
EcalVetoResult::~EcalVetoResult() { Clear(); }

void EcalVetoResult::Clear() {
  passes_veto_ = false;

  n_readout_hits_ = 0;
  summed_det_ = 0;
  summed_tight_iso_ = 0;
  max_cell_dep_ = 0;
  shower_rms_ = 0;
  x_std_ = 0;
  y_std_ = 0;
  avg_layer_hit_ = 0;
  std_layer_hit_ = 0;
  deepest_layer_hit_ = 0;
  ecal_back_energy_ = 0;
  n_tracking_hits_ = 0;
  ep_ang_ = 0;
  ep_ang_at_target_ = 0;
  ep_sep_ = 0;
  ep_dot_ = 0;
  ep_dot_at_target_ = 0;

  electron_containment_energy_.clear();
  photon_containment_energy_.clear();
  outside_containment_energy_.clear();
  outside_containment_n_hits_.clear();
  outside_containment_x_std_.clear();
  outside_containment_y_std_.clear();

  energy_seg_.clear();
  x_mean_seg_.clear();
  y_mean_seg_.clear();
  x_std_seg_.clear();
  y_std_seg_.clear();
  layer_mean_seg_.clear();
  layer_std_seg_.clear();

  e_cont_energy_.clear();
  e_cont_x_mean_.clear();
  e_cont_y_mean_.clear();
  g_cont_energy_.clear();
  g_cont_n_hits_.clear();
  g_cont_x_mean_.clear();
  g_cont_y_mean_.clear();
  o_cont_energy_.clear();
  o_cont_n_hits_.clear();
  o_cont_x_mean_.clear();
  o_cont_y_mean_.clear();
  o_cont_x_std_.clear();
  o_cont_y_std_.clear();
  o_cont_layer_mean_.clear();
  o_cont_layer_std_.clear();

  disc_value_ = 0;

  recoil_px_ = -9999;
  recoil_py_ = -9999;
  recoil_pz_ = -9999;
  recoil_x_ = -9999;
  recoil_y_ = -9999;

  ecal_layer_edep_readout_.clear();
}

void EcalVetoResult::setVariables(
    int n_readout_hits, int deepest_layer_hit, int n_tracking_hits,
    float summed_det, float summed_tight_iso, float max_cell_dep,
    float shower_rms, float x_std, float y_std, float avg_layer_hit,
    float std_layer_hit, float ecal_back_energy, float ep_ang,
    float ep_ang_at_target, float ep_sep, float ep_dot, float ep_dot_at_target,

    std::vector<float> electron_containment_energy,
    std::vector<float> photon_containment_energy,
    std::vector<float> outside_containment_energy,
    std::vector<int> outside_containment_n_hits,
    std::vector<float> outside_containment_x_std,
    std::vector<float> outside_containment_y_std,

    std::vector<float> energy_seg, std::vector<float> x_mean_seg,
    std::vector<float> y_mean_seg, std::vector<float> x_std_seg,
    std::vector<float> y_std_seg, std::vector<float> layer_mean_seg,
    std::vector<float> layer_std_seg,

    std::vector<std::vector<float>> e_cont_energy,
    std::vector<std::vector<float>> e_cont_x_mean,
    std::vector<std::vector<float>> e_cont_y_mean,
    std::vector<std::vector<float>> g_cont_energy,
    std::vector<std::vector<int>> g_cont_n_hits,
    std::vector<std::vector<float>> g_cont_x_mean,
    std::vector<std::vector<float>> g_cont_y_mean,
    std::vector<std::vector<float>> o_cont_energy,
    std::vector<std::vector<int>> o_cont_n_hits,
    std::vector<std::vector<float>> o_cont_x_mean,
    std::vector<std::vector<float>> o_cont_y_mean,
    std::vector<std::vector<float>> o_cont_x_std,
    std::vector<std::vector<float>> o_cont_y_std,
    std::vector<std::vector<float>> o_cont_layer_mean,
    std::vector<std::vector<float>> o_cont_layer_std,

    std::vector<float> ecal_layer_edep_readout, std::array<float, 3> recoil_p,
    std::array<float, 3> recoil_pos) {
  n_readout_hits_ = n_readout_hits;
  summed_det_ = summed_det;
  summed_tight_iso_ = summed_tight_iso;
  max_cell_dep_ = max_cell_dep;
  shower_rms_ = shower_rms;
  x_std_ = x_std;
  y_std_ = y_std;
  avg_layer_hit_ = avg_layer_hit;
  std_layer_hit_ = std_layer_hit;
  deepest_layer_hit_ = deepest_layer_hit;
  ecal_back_energy_ = ecal_back_energy;
  n_tracking_hits_ = n_tracking_hits;
  ep_ang_ = ep_ang;
  ep_ang_at_target_ = ep_ang_at_target;
  ep_sep_ = ep_sep;
  ep_dot_ = ep_dot;
  ep_dot_at_target_ = ep_dot_at_target;

  electron_containment_energy_ = electron_containment_energy;
  photon_containment_energy_ = photon_containment_energy;
  outside_containment_energy_ = outside_containment_energy;
  outside_containment_n_hits_ = outside_containment_n_hits;
  outside_containment_x_std_ = outside_containment_x_std;
  outside_containment_y_std_ = outside_containment_y_std;

  energy_seg_ = energy_seg;
  x_mean_seg_ = x_mean_seg;
  y_mean_seg_ = y_mean_seg;
  x_std_seg_ = x_std_seg;
  y_std_seg_ = y_std_seg;
  layer_mean_seg_ = layer_mean_seg;
  layer_std_seg_ = layer_std_seg;

  e_cont_energy_ = e_cont_energy;
  e_cont_x_mean_ = e_cont_x_mean;
  e_cont_y_mean_ = e_cont_y_mean;
  g_cont_energy_ = g_cont_energy;
  g_cont_n_hits_ = g_cont_n_hits;
  g_cont_x_mean_ = g_cont_x_mean;
  g_cont_y_mean_ = g_cont_y_mean;
  o_cont_energy_ = o_cont_energy;
  o_cont_n_hits_ = o_cont_n_hits;
  o_cont_x_mean_ = o_cont_x_mean;
  o_cont_y_mean_ = o_cont_y_mean;
  o_cont_x_std_ = o_cont_x_std;
  o_cont_y_std_ = o_cont_y_std;
  o_cont_layer_mean_ = o_cont_layer_mean;
  o_cont_layer_std_ = o_cont_layer_std;

  // discvalue not set here

  if (!recoil_p.empty()) {
    recoil_px_ = recoil_p[0];
    recoil_py_ = recoil_p[1];
    recoil_pz_ = recoil_p[2];
  }
  if (!recoil_pos.empty()) {
    recoil_x_ = recoil_pos[0];
    recoil_y_ = recoil_pos[1];
  }

  ecal_layer_edep_readout_ = ecal_layer_edep_readout;
}

std::ostream& operator<<(std::ostream& o, const EcalVetoResult& c) {
  return o << "EcalVetoResult { Passes veto : " << c.passes_veto_ << " }";
}
}  // namespace ldmx
