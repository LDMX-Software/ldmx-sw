
#include "DQM/HCalDQM.h"

#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalVetoResult.h"

namespace dqm {

HCalDQM::HCalDQM(const std::string& name, framework::Process& process)
    : framework::Analyzer(name, process) {}

void HCalDQM::configure(framework::config::Parameters& ps) {
  rec_coll_name_ = ps.getParameter<std::string>("rec_coll_name");
  rec_pass_name_ = ps.getParameter<std::string>("rec_pass_name");
  veto_name_ = ps.getParameter<std::string>("veto_name");
  veto_pass_ = ps.getParameter<std::string>("veto_pass");
}

void HCalDQM::analyze(const framework::Event& event) {
  // Get the collection of HCalDQM digitized hits if the exists
  const auto& hcalHits{event.getCollection<ldmx::HcalHit>(rec_coll_name_,rec_pass_name_)};

  float totalPE{0}, minTime{999}, minTimePE{-1};
  float maxPE{-1}, maxPETime{-1};

  // Loop through all HCal hits in the event
  // Get non-noise generated hits into new vector for sorting
  for (const ldmx::HcalHit& hit : hcalHits) {
    histograms_.fill("pe", hit.getPE());
    histograms_.fill("hit_time", hit.getTime());

    totalPE += hit.getPE();

    // PE veto threshold set at 5
    if (hit.getTime() > -999. and hit.getPE() > 5. and hit.getTime() < minTime) {
      minTime   = hit.getTime();
      minTimePE = hit.getPE();
    }

    if (hit.getPE() > maxPE) {
      maxPE = hit.getPE();
      maxPETime = hit.getTime();
    }
  }

  // let's fill our once-per-event histograms
  histograms_.fill("n_hits", hcalHits.size());
  histograms_.fill("total_pe", totalPE);
  histograms_.fill("max_pe", maxPE);
  histograms_.fill("hit_time_max_pe", maxPETime);
  histograms_.fill("max_pe:time", maxPE, maxPETime);
  histograms_.fill("min_time_hit_above_thresh", minTime);
  histograms_.fill("min_time_hit_above_thresh:pe", minTimePE, minTime);

}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalDQM)
