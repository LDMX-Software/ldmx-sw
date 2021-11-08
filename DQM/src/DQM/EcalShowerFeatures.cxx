
#include "DQM/EcalShowerFeatures.h"

#include "Ecal/Event/EcalVetoResult.h"

namespace dqm {

void EcalShowerFeatures::configure(framework::config::Parameters &ps) {
  ecal_veto_name_ = ps.getParameter<std::string>("ecal_veto_name");
  ecal_veto_pass_ = ps.getParameter<std::string>("ecal_veto_pass");

  return;
}

void EcalShowerFeatures::analyze(const framework::Event &event) {
  auto veto{event.getObject<ldmx::EcalVetoResult>(ecal_veto_name_,
                                                  ecal_veto_pass_)};

  histograms_.fill("deepest_layer_hit", veto.getDeepestLayerHit());
  histograms_.fill("num_readout_hits", veto.getNReadoutHits());
  histograms_.fill("summed_det", veto.getSummedDet());
  histograms_.fill("summed_iso", veto.getSummedTightIso());
  histograms_.fill("summed_back", veto.getEcalBackEnergy());
  histograms_.fill("max_cell_dep", veto.getMaxCellDep());
  histograms_.fill("shower_rms", veto.getShowerRMS());
  histograms_.fill("x_std", veto.getXStd());
  histograms_.fill("y_std", veto.getYStd());
  histograms_.fill("avg_layer_hit", veto.getAvgLayerHit());
  histograms_.fill("std_layer_hit", veto.getStdLayerHit());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, EcalShowerFeatures);
