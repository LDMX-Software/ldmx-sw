
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
  
  //pass/fail is a boolean, simple binning
  histograms_.create(trigger_collName_+".pass","Pass trigger",2,0,2);

  int maxE = 20000;
  int minE = 0;
  int nBinsE = (maxE-minE)/100;
  histograms_.create(trigger_collName_+".EcalEsum","Ecal energy sum [MeV]",nBinsE,minE,maxE);
  histograms_.create(trigger_collName_+".EcalEcut","Ecal E_{max} cut [MeV]",nBinsE,minE,maxE);
  
  int maxLayer = 32;
  int minLayer = 0;
  int nBinsLayer = (maxLayer-minLayer);
  histograms_.create(trigger_collName_+".EcalLayercut","Ecal layer_{max}",nBinsLayer,minLayer,maxLayer);
  
  int maxElectrons = 10;
  int minElectrons = 0;
  int nBinsElectrons = (maxElectrons-minElectrons);
  histograms_.create(trigger_collName_+".nElectrons","N_{electrons} used for trigger decision",nBinsElectrons,minElectrons,maxElectrons);
}

void Trigger::analyze(const framework::Event &event) {

  auto trigResult{event.getObject<ldmx::TriggerResult>(trigger_collName_,
                                                  trigger_passName_)};

  histograms_.fill(trigger_collName_+".EcalEsum", trigResult.getAlgoVar(0) );
  histograms_.fill(trigger_collName_+".EcalEcut", trigResult.getAlgoVar(1) );
  histograms_.fill(trigger_collName_+".EcalLayercut", trigResult.getAlgoVar(2) );
  histograms_.fill(trigger_collName_+".nElectrons", trigResult.getAlgoVar(3) );
  histograms_.fill(trigger_collName_+".pass", trigResult.passed() );
  
  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, Trigger);
