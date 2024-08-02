#include "Trigger/PropagationMapWriter.h"

#include "SimCore/Event/SimTrackerHit.h"
// #include "Trigger/Event/TrigParticle.h"
// #include "Trigger/Event/TrigEnergySum.h"

namespace trigger {
PropagationMapWriter::PropagationMapWriter(const std::string& name,
               framework::Process& process)
    : Producer(name, process) {}

void PropagationMapWriter::configure(framework::config::Parameters& ps) {
  outPath_ = ps.getParameter<std::string>("outPath");
  targetSPName_  = ps.getParameter<std::string>("inputTargetSPName", "TargetScoringPlaneHits"); 
  ecalSPName_    = ps.getParameter<std::string>("inputEcalSPName", "EcalScoringPlaneHits"); 
  targetSPz_  = ps.getParameter<double>("inputTargetSPz", 0.18); 
  ecalSPz_    = ps.getParameter<double>("inputEcalSPz", 240); 
}

void PropagationMapWriter::produce(framework::Event& event) {

  if (!event.exists(targetSPName_)) return;
  const std::vector<ldmx::SimTrackerHit> hitsTarg = event.getCollection<ldmx::SimTrackerHit>(targetSPName_);
  if (!event.exists(ecalSPName_)) return;
  const std::vector<ldmx::SimTrackerHit> hitsEcal = event.getCollection<ldmx::SimTrackerHit>(ecalSPName_);
  
  ldmx::SimTrackerHit h1, h2; // the desired truth hits
  for(const auto& hit : hitsTarg){
    if(!(hit.getTrackID()==1)) continue;
    if(!(hit.getPdgID()==11)) continue;
    auto xyz = hit.getPosition();
    if( fabs(targetSPz_-xyz[2])>0.1 ) continue; // select one sp
    h1 = hit;
  }
  for(const auto& hit : hitsEcal){
    if(!(hit.getTrackID()==1)) continue;
    if(!(hit.getPdgID()==11)) continue;
    auto xyz = hit.getPosition();
    if( fabs(ecalSPz_-xyz[2])>0.01 ) continue; // select one sp
    h2 = hit;
  }

  // std::cout << h1.getPdgID() << " and " <<  h2.getPdgID() << std::endl;

  if( h1.getPdgID() && h2.getPdgID() && h1.getEnergy() && fabs(h2.getEnergy()/h1.getEnergy()-1)<0.01 ){
    // as a function of the Ecal face electron (but this should make a minimal difference)
    profx_->Fill(h2.getEnergy(), h1.getEnergy() ? h1.getMomentum()[0]/h1.getEnergy() : 0, h2.getPosition()[0]-h1.getPosition()[0]);
    profy_->Fill(h2.getEnergy(), h1.getEnergy() ? h1.getMomentum()[1]/h1.getEnergy() : 0, h2.getPosition()[1]-h1.getPosition()[1]);
    lutx_->Fill(h2.getEnergy(), h2.getPosition()[0]-h1.getPosition()[0], h1.getEnergy() ? h1.getMomentum()[0]/h1.getEnergy() : 0);
    luty_->Fill(h2.getEnergy(), h2.getPosition()[1]-h1.getPosition()[1], h1.getEnergy() ? h1.getMomentum()[1]/h1.getEnergy() : 0);
    if (fabs(h1.getMomentum()[0])>cut_){
      profxCut_->Fill(h2.getEnergy(), h1.getEnergy() ? h1.getMomentum()[0]/h1.getEnergy() : 0, h2.getPosition()[0]-h1.getPosition()[0]);
      lutxCut_->Fill(h2.getEnergy(), h2.getPosition()[0]-h1.getPosition()[0], h1.getEnergy() ? h1.getMomentum()[0]/h1.getEnergy() : 0);
    }
    if (fabs(h1.getMomentum()[1])>cut_){
      profyCut_->Fill(h2.getEnergy(), h1.getEnergy() ? h1.getMomentum()[1]/h1.getEnergy() : 0, h2.getPosition()[1]-h1.getPosition()[1]);
      lutyCut_->Fill(h2.getEnergy(), h2.getPosition()[1]-h1.getPosition()[1], h1.getEnergy() ? h1.getMomentum()[1]/h1.getEnergy() : 0);
    }
    de_->Fill(h1.getEnergy(), h1.getEnergy() - h2.getEnergy());
    // profx_->Fill(h1.getEnergy(), h1.getMomentum()[0]/h1.getEnergy(), h2.getPosition()[0]-h1.getPosition()[0]);
    // profy_->Fill(h1.getEnergy(), h1.getMomentum()[1]/h1.getEnergy(), h2.getPosition()[1]-h1.getPosition()[1]);
  }
}

void PropagationMapWriter::onProcessStart() {
  // auto hdir = getHistoDirectory();
  outFile_ = new TFile(outPath_.c_str(),"recreate");
  outFile_->SetCompressionSettings(209);
  // 100*alg+level
  // 2=LZMA, 9 = max compression
  profx_ = new TProfile2D("profx",";E_{ECal};px/e;dx",40,0,4000,40,-1,1,-200,200);
  profy_ = new TProfile2D("profy",";E_{ECal};py/e;dy",40,0,4000,40,-1,1,-200,200); 
  lutx_ = new TProfile2D("lutx",";E_{ECal};dx;px/e",40,0,4000,40,-200,200,-1,1);
  luty_ = new TProfile2D("luty",";E_{ECal};dy;py/e",40,0,4000,40,-200,200,-1,1); 
  profxCut_ = new TProfile2D("cutprofx",";E_{ECal};px/e;dx",40,0,4000,40,-1,1,-200,200);
  profyCut_ = new TProfile2D("cutprofy",";E_{ECal};py/e;dy",40,0,4000,40,-1,1,-200,200); 
  lutxCut_ = new TProfile2D("cutlutx",";E_{ECal};dx;px/e",40,0,4000,40,-200,200,-1,1);
  lutyCut_ = new TProfile2D("cutluty",";E_{ECal};dy;py/e",40,0,4000,40,-200,200,-1,1); 
  de_ = new TH2D("de",";E_{Target};E_{Target}-E_{ECal}",40,0,4000,40,-100,500); 
}

void PropagationMapWriter::onProcessEnd() {
  outFile_->Write();
  outFile_->Close();
}

}  // namespace trigger
DECLARE_PRODUCER_NS(trigger, PropagationMapWriter);
