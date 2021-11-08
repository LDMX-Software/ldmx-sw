
#include "DQM/TrigScintDQM.h"

#include "SimCore/Event/SimCalorimeterHit.h"

namespace dqm {

TrigScintDQM::TrigScintDQM(const std::string &name, framework::Process &process)
    : framework::Analyzer(name, process) {}

TrigScintDQM::~TrigScintDQM() {}

void TrigScintDQM::onProcessStart() {
  std::cout << "Process starts!" << std::endl;

  getHistoDirectory();

  histograms_.create("id", "Channel ID of sim hit", 100, 0, 100);
  histograms_.create("total_energy", "Total energy deposition in the pad/event",
                     1000, 0, 3000);
  histograms_.create("n_hits", "TrigScint hit multiplicity in the pad/event",
                     100, 0, 100);
  histograms_.create("x", "Hit x position", 1000, -100, 100);
  histograms_.create("y", "Hit y position", 1000, -100, 100);
  histograms_.create("z", "Hit z position", 1000, -900, 100);

  histograms_.create("energy", "Energy deposition in a TrigScint bar", 250, 0,
                     1500);
  histograms_.create("hit_time", "TrigScint hit time (ns)", 1600, -100, 1500);

  histograms_.create("max_pe:time", "Max Photoelectrons in a TrigScint bar",
                     1500, 0, 1500, "TrigScint max PE hit time (ns)", 1500, 0,
                     1500);

  histograms_.create("min_time_hit_above_thresh:pe",
                     "Photoelectrons in a TrigScint bar", 1500, 0, 1500,
                     "Earliest time of TrigScint hit above threshold (ns)",
                     1600, -100, 1500);
}

void TrigScintDQM::configure(framework::config::Parameters &ps) {
  hitCollectionName_ = ps.getParameter<std::string>("hit_collection");
  padName_ = ps.getParameter<std::string>("pad");

  std::cout << "In TrigScintDQM::configure, got parameters "
            << hitCollectionName_ << " and " << padName_ << std::endl;
}

void TrigScintDQM::analyze(const framework::Event &event) {
  const std::vector<ldmx::SimCalorimeterHit> TrigScintHits =
      event.getCollection<ldmx::SimCalorimeterHit>(hitCollectionName_);

  // Get the total hit count
  int hitCount = TrigScintHits.size();
  histograms_.fill("n_hits", hitCount);

  double totalEnergy{0};
  for (const ldmx::SimCalorimeterHit &hit : TrigScintHits) {
    ldmx::TrigScintID detID(hit.getID());

    int bar = detID.bar();

    histograms_.fill("energy", hit.getEdep());
    histograms_.fill("hit_time", hit.getTime());
    histograms_.fill("id", bar);

    std::vector<float> posvec = hit.getPosition();
    histograms_.fill("x", posvec.at(0));
    histograms_.fill("y", posvec.at(1));
    histograms_.fill("z", posvec.at(2));

    totalEnergy += hit.getEdep();
  }

  histograms_.fill("total_energy", totalEnergy);
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, TrigScintDQM)
