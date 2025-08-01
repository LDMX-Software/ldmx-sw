#include "Hcal/Event/VisiblesVetoResult.h"

ClassImp(ldmx::VisiblesVetoResult);

namespace ldmx {
VisiblesVetoResult::VisiblesVetoResult() {}

VisiblesVetoResult::~VisiblesVetoResult() { Clear(); }

void VisiblesVetoResult::Clear() {
  passes_veto_ = false;

  n_layers_hit_ = 0;
  x_std_ = 0.;
  y_std_ = 0.;
  z_std_ = 0.;
  x_mean_ = 0.;
  y_mean_ = 0.;
  r_mean_ = 0.;
  iso_hits_ = 0;
  iso_energy_ = 0.;
  n_readout_hits_ = 0;
  summed_det_ = 0.;
  r_mean_from_photon_track_ = 0.;

  disc_value_ = 0.;
}

void VisiblesVetoResult::setVariables(int n_layers_hit, double x_std,
                                      double y_std, double z_std, double x_mean,
                                      double y_mean, double r_mean,
                                      int iso_hits, double iso_energy,
                                      int n_readout_hits, double summed_det,
                                      double r_mean_from_photon_track) {
  n_layers_hit_ = n_layers_hit;
  x_std_ = x_std;
  y_std_ = y_std;
  z_std_ = z_std;
  x_mean_ = x_mean;
  y_mean_ = y_mean;
  r_mean_ = r_mean;
  iso_hits_ = iso_hits;
  iso_energy_ = iso_energy;
  n_readout_hits_ = n_readout_hits;
  summed_det_ = summed_det;
  r_mean_from_photon_track_ = r_mean_from_photon_track;
}

void VisiblesVetoResult::Print() const {
  std::cout << "[ VisiblesVetoResult ]:\n"
            << "\t Passes veto : " << passesVeto_ << "\n"
            << std::endl;
}
}  // namespace ldmx
