
#include "DQM/Trigger.h"

#include "Recon/Event/TriggerResult.h"

namespace dqm {

void Trigger::configure(framework::config::Parameters &ps) {
  trigger_collName_ = ps.getParameter<std::string>("trigger_name");
  trigger_passName_ = ps.getParameter<std::string>("trigger_pass");

  return;
}

void Trigger::onProcessStart() {
  getHistoDirectory();
  // create trigger variable histograms

  // pass/fail is a boolean, simple binning
  histograms_.create(trigger_collName_ + ".pass", "Pass trigger", 2, 0, 2);

  int max_e = 20000;
  int min_e = 0;
  int n_bins_e = (max_e - min_e) / 100;
  histograms_.create(trigger_collName_ + ".EcalEsum", "Ecal energy sum [MeV]",
                     n_bins_e, min_e, max_e);
  histograms_.create(trigger_collName_ + ".EcalEcut", "Ecal E_{max} cut [MeV]",
                     n_bins_e, min_e, max_e);

  int max_layer = 32;
  int min_layer = 0;
  int n_bins_layer = (max_layer - min_layer);
  histograms_.create(trigger_collName_ + ".EcalLayercut", "Ecal layer_{max}",
                     n_bins_layer, min_layer, max_layer);

  int max_electrons = 10;
  int min_electrons = 0;
  int n_bins_electrons = (max_electrons - min_electrons);
  histograms_.create(trigger_collName_ + ".nElectrons",
                     "N_{electrons} used for trigger decision", n_bins_electrons,
                     min_electrons, max_electrons);
}

void Trigger::analyze(const framework::Event &event) {
  auto trig_result{event.getObject<ldmx::TriggerResult>(trigger_collName_,
                                                       trigger_passName_)};

  histograms_.fill(trigger_collName_ + ".EcalEsum", trig_result.getAlgoVar(0));
  histograms_.fill(trigger_collName_ + ".EcalEcut", trig_result.getAlgoVar(1));
  histograms_.fill(trigger_collName_ + ".EcalLayercut",
                   trig_result.getAlgoVar(2));
  histograms_.fill(trigger_collName_ + ".nElectrons", trig_result.getAlgoVar(3));
  histograms_.fill(trigger_collName_ + ".pass", trig_result.passed());

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::Trigger);
