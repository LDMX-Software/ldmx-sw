#include "DQM/TrigScintHitDQM.h"

namespace dqm {

TrigScintHitDQM::TrigScintHitDQM(const std::string &name,
                                 framework::Process &process)
    : framework::Analyzer(name, process) {}

void TrigScintHitDQM::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  getHistoDirectory();

  histograms_.create("id", "Channel ID of sim hit", 100, 0, 100);
  histograms_.create("total_pe", "Total pe deposition in the pad/event", 500, 0,
                     2000);
  histograms_.create("n_hits", "TrigScint hit multiplicity in the pad/event",
                     100, 0, 100);
  histograms_.create("x", "Hit x position", 1000, -100, 100);
  histograms_.create("y", "Hit y position", 1000, -100, 100);
  histograms_.create("z", "Hit z position", 1000, -900, 100);

  histograms_.create("pe", "Pe deposition in a TrigScint bar", 250, 0, 1000);
  histograms_.create("hit_time", "TrigScint hit time (ns)", 600, -150, 150);

  histograms_.create("id_noise", "Channel ID of noise hit", 101, -1, 100);
  histograms_.create("pe_noise", "Pe deposition in a TrigScint bar noise hit",
                     100, 0, 100);
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
  hit_collection_name_ = ps.get<std::string>("hit_collection");
  pad_name_ = ps.get<std::string>("pad").c_str();

  trig_scint_passname_ = ps.get<std::string>("trig_scint_passname");

  ldmx_log(debug) << "In TrigScintHitDQM::configure, got parameters "
                  << hit_collection_name_ << " and " << pad_name_;
}

void TrigScintHitDQM::analyze(const framework::Event &event) {
  // Get the collection of TrigScintHit digitized hits if the exists
  const std::vector<ldmx::TrigScintHit> trig_scint_hits =
      event.getCollection<ldmx::TrigScintHit>(hit_collection_name_,
                                              trig_scint_passname_);

  // Get the total hit count
  int hit_count = trig_scint_hits.size();
  histograms_.fill("n_hits", hit_count);

  double total_pe{0};
  int noise_hit_count = 0;

  ldmx_log(debug) << "Looping over hits in " << hit_collection_name_;

  // Loop through all TrigScint hits in the event

  for (const ldmx::TrigScintHit &hit : trig_scint_hits) {
    histograms_.fill("pe", hit.getPE());
    histograms_.fill("hit_time", hit.getTime());
    histograms_.fill("id", hit.getBarID());

    total_pe += hit.getPE();
    if (hit.isNoise() > 0) {
      noise_hit_count++;
      histograms_.fill("pe_noise", hit.getPE());
      histograms_.fill("id_noise", hit.getBarID());
    } else {  // x, y, z not set for noise hits
      histograms_.fill("x", hit.getXPos());
      histograms_.fill("y", hit.getYPos());
      histograms_.fill("z", hit.getZPos());
    }
  }

  histograms_.fill("total_pe", total_pe);
  histograms_.fill("n_hits_noise", noise_hit_count);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::TrigScintHitDQM)
