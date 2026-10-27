#include "DQM/EventSummaryDQM.h"

#include "Framework/EventSummary.h"

namespace dqm {

void EventSummaryDQM::configure(framework::config::Parameters& ps) {
  summary_name_ = ps.get<std::string>("summary_name");
  summary_pass_ = ps.get<std::string>("summary_pass");
}

void EventSummaryDQM::onProcessStart() {
  getHistoDirectory();
  histograms_.create("n_systems", "Subsystems in the event", 8, 0, 8);
  histograms_.create("system_id", "Subsystem ID", 16, 0, 16);
  histograms_.create("error_bit", "Error flag bit (-1 = none)", 8, -1, 7);
  histograms_.create("payload_kb", "Event payload [kB]", 100, 0, 500);
  histograms_.create("dt_ms", "Time since previous event [ms]", 100, 0, 100);
}

void EventSummaryDQM::analyze(const framework::Event& event) {
  // the builder adds nothing once its input is exhausted
  if (!event.exists(summary_name_, summary_pass_)) return;
  auto summary{
      event.getObject<ldmx::EventSummary>(summary_name_, summary_pass_)};

  histograms_.fill("n_systems", summary.getNSystems());
  for (auto id : summary.getSystemIds()) histograms_.fill("system_id", id);

  const uint32_t flags = summary.getErrorFlags();
  if (flags == 0) histograms_.fill("error_bit", -1);
  for (int bit{0}; bit < 7; bit++) {
    if (flags & (1u << bit)) histograms_.fill("error_bit", bit);
  }

  histograms_.fill("payload_kb", summary.getPayloadSize() / 1024.);

  const uint64_t ts = summary.getTimestampNs();
  if (last_timestamp_ > 0 and ts > last_timestamp_) {
    histograms_.fill("dt_ms", (ts - last_timestamp_) / 1e6);
  }
  last_timestamp_ = ts;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EventSummaryDQM);
