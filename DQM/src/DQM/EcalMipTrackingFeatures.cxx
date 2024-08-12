
#include "DQM/EcalMipTrackingFeatures.h"

#include "Ecal/Event/EcalVetoResult.h"

namespace dqm {

void EcalMipTrackingFeatures::configure(framework::config::Parameters &ps) {
  ecal_veto_name_ = ps.getParameter<std::string>("ecal_veto_name");
  ecal_veto_pass_ = ps.getParameter<std::string>("ecal_veto_pass");

  return;
}

void EcalMipTrackingFeatures::analyze(const framework::Event &event) {
  const auto &veto{
      event.getObject<ldmx::EcalVetoResult>(ecal_veto_name_, ecal_veto_pass_)};

  histograms_.fill("n_straight_tracks", veto.getNStraightTracks());
  histograms_.fill("n_linreg_tracks", veto.getNLinRegTracks());
  histograms_.fill("first_near_photon_layer", veto.getFirstNearPhLayer());
  histograms_.fill("ep_ang", veto.getEPAng());
  histograms_.fill("ep_sep", veto.getEPSep());
  auto recoil_mom = veto.getRecoilMomentum();
  histograms_.fill("recoil_pz", recoil_mom[2]);
  histograms_.fill("recoil_pt", std::sqrt(recoil_mom[0] * recoil_mom[0] +
                                          recoil_mom[1] * recoil_mom[1]));
  histograms_.fill("recoil_x", veto.getRecoilX());
  histograms_.fill("recoil_y", veto.getRecoilY());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, EcalMipTrackingFeatures);
