#include "Trigger/NtupleWriter.h"

namespace trigger {
NtupleWriter::NtupleWriter(const std::string& name,
               framework::Process& process)
    : Producer(name, process) {}

void NtupleWriter::configure(framework::config::Parameters& ps) {
  std::cout << " NtupleWriter::cfg " << std::endl;
}

void NtupleWriter::produce(framework::Event& event) {
  std::cout << " NtupleWriter::produce " << std::endl;
  framework::NtupleManager& n{framework::NtupleManager::getInstance()};
  n.setVar<int>("nTargetHitPrimary", 7);
}

void NtupleWriter::onProcessStart() {
  // auto hdir = getHistoDirectory();
  outFile_ = new TFile("ntuple.root","recreate");
  framework::NtupleManager& n{framework::NtupleManager::getInstance()};

  n.create("Analysis"); 
  n.addVar<int> ("Analysis", "nTargetHitPrimary");

  // hdir->cd("../");
  // eventTree_= new TTree("Events","EventTree");
  // hdir->GetFile()->SetCompressionSettings(209); 
  // 100*alg+level
  // 2=LZMA, 9 = max compression
  // eventTree_->Branch("Target_Proton_pz", &Target_Proton_pz);
  
}
  void NtupleWriter::onProcessEnd() {
    outFile_->Write();
    outFile_->Close();
  }

}  // namespace trigger
DECLARE_PRODUCER_NS(trigger, NtupleWriter);
