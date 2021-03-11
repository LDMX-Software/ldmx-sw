/**
 * @file TrigScintHitDQM.cxx
 * @brief Analyzer used for TrigScint Digi DQM.
 * @author Omar Moreno, SLAC National Accelerator
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "DQM/TrigScintHitDQM.h"

namespace dqm {

TrigScintHitDQM::TrigScintHitDQM(const std::string &name,
                                 framework::Process &process)
    : framework::Analyzer(name, process) {}

TrigScintHitDQM::~TrigScintHitDQM() {}

void TrigScintHitDQM::onProcessStart() {
  std::cout << "Process starts!" << std::endl;

  getHistoDirectory();

  histograms_.create("id", "Channel ID of sim hit", 100, 0, 100);
  histograms_.create("total_pe", "Total pe deposition in the pad/event", 2000,
                     0, 2000);
  histograms_.create("n_hits", "TrigScint hit multiplicity in the pad/event",
                     100, 0, 100);
  histograms_.create("x", "Hit x position", 1000, -100, 100);
  histograms_.create("y", "Hit y position", 1000, -100, 100);
  histograms_.create("z", "Hit z position", 1000, -900, 100);

  histograms_.create("pe", "Pe deposition in a TrigScint bar", 1500, 0, 1500);
  histograms_.create("hit_time", "TrigScint hit time (ns)", 1600, -100, 1500);

  histograms_.create("id_noise", "Channel ID of noise hit", 100, 0, 100);
  histograms_.create("pe_noise", "Pe deposition in a TrigScint bar noise hit",
                     1500, 0, 1500);
  histograms_.create("n_hits_noise",
                     "TrigScint noise hit multiplicity in the pad/event", 100,
                     0, 100);

  histograms_.create("max_pe:time", "Max Photoelectrons in a TrigScint bar",
                     1500, 0, 1500, "TrigScint max PE hit time (ns)", 1500, 0,
                     1500);

  histograms_.create("min_time_hit_above_thresh:pe",
                     "Photoelectrons in a TrigScint bar", 1500, 0, 1500,
                     "Earliest time of TrigScint hit above threshold (ns)",
                     1600, -100, 1500);

  // TODO: implement getting a list of the constructed histograms, to iterate
  // through and set overflow boolean.
}

void TrigScintHitDQM::configure(framework::config::Parameters &ps) {
  hitCollectionName_ = ps.getParameter<std::string>("hit_collection");
  padName_ = ps.getParameter<std::string>("pad").c_str();

  std::cout << "In TrigScintHitDQM::configure, got parameters "
            << hitCollectionName_ << " and " << padName_ << std::endl;
}

void TrigScintHitDQM::analyze(const framework::Event &event) {
  // Get the collection of TrigScintHit digitized hits if the exists
  const std::vector<ldmx::TrigScintHit> TrigScintHits =
      event.getCollection<ldmx::TrigScintHit>(hitCollectionName_);

  // Get the total hit count
  int hitCount = TrigScintHits.size();
  histograms_.fill("n_hits", hitCount);

  double totalPE{0};
  int noiseHitCount = 0;

  // Loop through all TrigScint hits in the event

  for (const ldmx::TrigScintHit &hit : TrigScintHits) {
    histograms_.fill("pe", hit.getPE());
    histograms_.fill("hit_time", hit.getTime());
    histograms_.fill("id", hit.getBarID());

    totalPE += hit.getPE();
    if (hit.isNoise() > 0) {
      noiseHitCount++;
      histograms_.fill("pe_noise", hit.getPE());
      histograms_.fill("id_noise", hit.getBarID());
    } else {  // x, y, z not set for noise hits
      histograms_.fill("x", hit.getXPos());
      histograms_.fill("y", hit.getYPos());
      histograms_.fill("z", hit.getZPos());
    }
  }

  histograms_.fill("total_pe", totalPE);
  histograms_.fill("n_hits_noise", noiseHitCount);
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, TrigScintHitDQM)
