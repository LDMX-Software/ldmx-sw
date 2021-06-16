
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

  // Get the total hit count
  int hitCount = hcalHits.size();
  histograms_.fill("n_hits", hitCount);

  double totalPE{0};

  // Loop through all HCal hits in the event
  // Get non-noise generated hits into new vector for sorting
  std::vector<const ldmx::HcalHit*> filteredHits;
  for (const ldmx::HcalHit& hit : hcalHits) {
    histograms_.fill("pe", hit.getPE());
    histograms_.fill("hit_time", hit.getTime());

    totalPE += hit.getPE();

    if (hit.getTime() > -999.) {
      filteredHits.push_back(&hit);
    }
  }

  histograms_.fill("total_pe", totalPE);

  // Sort the array by hit time
  std::sort(filteredHits.begin(), filteredHits.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs->getTime() < rhs->getTime();
            });

  // get first time and PE of hit over threshold
  //  hardcode threshold to 5PE to match veto
  double minTime{-1};
  double minTimePE{-1};
  for (const auto& hit : filteredHits) {
    if (hit->getPE() < 5.) continue;
    minTime = hit->getTime();
    minTimePE = hit->getPE();
    break;
  }

  histograms_.fill("min_time_hit_above_thresh", minTime);
  histograms_.fill("min_time_hit_above_thresh:pe", minTimePE, minTime);

  float maxPE{-1};
  float maxPETime{-1};
  bool passesHcalVeto{false};
  // Check if the HcalVeto result exists
  if (event.exists(veto_name_,veto_pass_)) {
    // Get the collection of HCalDQM digitized hits if the exists
    const auto& hcalVeto{event.getObject<ldmx::HcalVetoResult>(veto_name_,veto_pass_)};

    ldmx::HcalHit maxPEHit = hcalVeto.getMaxPEHit();

    // Get the max PE and it's time
    maxPE = maxPEHit.getPE();
    maxPETime = maxPEHit.getTime();

    histograms_.fill("max_pe", maxPE);
    histograms_.fill("hit_time_max_pe", maxPETime);
    histograms_.fill("max_pe:time", maxPE, maxPETime);
    histograms_.fill("veto", hcalVeto.passesVeto());

    if (hcalVeto.passesVeto()) {
      histograms_.fill("max_pe_hcal_veto", maxPE);
      histograms_.fill("hit_time_max_pe_hcal_veto", maxPETime);
      histograms_.fill("max_pe:time_hcal_veto", maxPE, maxPETime);
      histograms_.fill("total_pe_hcal_veto", totalPE);
      histograms_.fill("n_hits_hcal_veto", hitCount);
      passesHcalVeto = true;
    }
  }
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, HCalDQM)
