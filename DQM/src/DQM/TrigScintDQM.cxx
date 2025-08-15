
#include "DQM/TrigScintDQM.h"

#include "SimCore/Event/SimCalorimeterHit.h"

namespace dqm {

TrigScintDQM::TrigScintDQM(const std::string &name, framework::Process &process)
    : framework::Analyzer(name, process) {}

void TrigScintDQM::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  getHistoDirectory();

  histograms_.create("id", "Channel ID of sim hit", 100, 0, 100);
  histograms_.create("total_energy",
                     "Total energy deposition in the pad/event [MeV]", 1000, 0,
                     3000);
  histograms_.create("n_hits", "Hit multiplicity in the pad/event [MeV]", 100,
                     0, 100);
  histograms_.create("x", "Hit x position [mm]", 1000, -100, 100);
  histograms_.create("y", "Hit y position [mm]", 1000, -100, 100);
  histograms_.create("z", "Hit z position", 1000, -900, 100);

  histograms_.create("energy", "Energy deposition in a bar", 250, 0, 1500);
  histograms_.create("hit_time", "Hit time [ns]", 1600, -100, 1500);

  histograms_.create("max_pe:time", "Max Photoelectrons in a bar", 1500, 0,
                     1500, "Max PE hit time [ns]", 1500, 0, 1500);

  histograms_.create("min_time_hit_above_thresh:pe", "Photoelectrons in a bar",
                     1500, 0, 1500, "Earliest time of hit above threshold [ns]",
                     1600, -100, 1500);
}

void TrigScintDQM::configure(framework::config::Parameters &ps) {
  hit_collection_name_ = ps.get<std::string>("hit_collection");
  pad_name_ = ps.get<std::string>("pad");
  hit_passname_ = ps.get<std::string>("hit_passname");

  ldmx_log(debug) << " Collection name = " << hit_collection_name_
                  << " pad name = " << pad_name_;
}

void TrigScintDQM::analyze(const framework::Event &event) {
  const std::vector<ldmx::SimCalorimeterHit> trig_scint_hits =
      event.getCollection<ldmx::SimCalorimeterHit>(hit_collection_name_,
                                                   hit_passname_);

  // Get the total hit count
  int hit_count = trig_scint_hits.size();
  histograms_.fill("n_hits", hit_count);

  double total_energy{0};
  for (const ldmx::SimCalorimeterHit &hit : trig_scint_hits) {
    ldmx::TrigScintID det_id(hit.getID());

    int bar = det_id.bar();

    histograms_.fill("energy", hit.getEdep());
    histograms_.fill("hit_time", hit.getTime());
    histograms_.fill("id", bar);

    std::vector<float> posvec = hit.getPosition();
    histograms_.fill("x", posvec.at(0));
    histograms_.fill("y", posvec.at(1));
    histograms_.fill("z", posvec.at(2));

    total_energy += hit.getEdep();
  }

  histograms_.fill("total_energy", total_energy);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::TrigScintDQM)
