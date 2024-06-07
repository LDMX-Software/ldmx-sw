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
}

void PropagationMapWriter::produce(framework::Event& event) {
  std::string inTag;
  inTag = "TargetScoringPlaneHits";
  if (!event.exists(inTag)) return;
  const std::vector<ldmx::SimTrackerHit> hitsTarg =
      event.getCollection<ldmx::SimTrackerHit>(inTag);

  inTag = "EcalScoringPlaneHits";
  if (!event.exists(inTag)) return;
  const std::vector<ldmx::SimTrackerHit> hitsEcal =
      event.getCollection<ldmx::SimTrackerHit>(inTag);

  ldmx::SimTrackerHit h1, h2;  // the desired truth hits
  for (const auto& hit : hitsTarg) {
    if (!(hit.getTrackID() == 1)) continue;
    if (!(hit.getPdgID() == 11)) continue;
    auto xyz = hit.getPosition();
    if (xyz[2] < 0 || xyz[2] > 1) continue;  // select one sp
    h1 = hit;
  }
  for (const auto& hit : hitsEcal) {
    if (!(hit.getTrackID() == 1)) continue;
    if (!(hit.getPdgID() == 11)) continue;
    auto xyz = hit.getPosition();
    if (xyz[2] < 239.99 || xyz[2] > 240.01) continue;
    h2 = hit;
  }

  // std::cout << h1.getPdgID() << " and " <<  h2.getPdgID() << std::endl;

  if (h1.getPdgID() && h2.getPdgID()) {
    // as a function of the Ecal face electron (but this should make a minimal
    // difference)
    profx_->Fill(h2.getEnergy(), h1.getMomentum()[0] / h1.getEnergy(),
                 h2.getPosition()[0] - h1.getPosition()[0]);
    profy_->Fill(h2.getEnergy(), h1.getMomentum()[1] / h1.getEnergy(),
                 h2.getPosition()[1] - h1.getPosition()[1]);
    // profx_->Fill(h1.getEnergy(), h1.getMomentum()[0]/h1.getEnergy(),
    // h2.getPosition()[0]-h1.getPosition()[0]); profy_->Fill(h1.getEnergy(),
    // h1.getMomentum()[1]/h1.getEnergy(),
    // h2.getPosition()[1]-h1.getPosition()[1]);
  }
}

void PropagationMapWriter::onProcessStart() {
  // auto hdir = getHistoDirectory();
  outFile_ = new TFile(outPath_.c_str(), "recreate");
  outFile_->SetCompressionSettings(209);
  // 100*alg+level
  // 2=LZMA, 9 = max compression
  profx_ = new TProfile2D("profx", ";energy;px/e", 40, 0, 4000, 40, -1, 1, -200,
                          200);
  profy_ = new TProfile2D("profy", ";energy;py/e", 40, 0, 4000, 40, -1, 1, -200,
                          200);
}

void PropagationMapWriter::onProcessEnd() {
  outFile_->Write();
  outFile_->Close();
}

}  // namespace trigger
DECLARE_PRODUCER_NS(trigger, PropagationMapWriter);
