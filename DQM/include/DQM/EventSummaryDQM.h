#ifndef DQM_EVENTSUMMARYDQM_H
#define DQM_EVENTSUMMARYDQM_H

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * @class EventSummaryDQM
 * @brief Histograms of the EventBuilder's EventSummary
 */
class EventSummaryDQM : public framework::Analyzer {
 public:
  EventSummaryDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  void configure(framework::config::Parameters& ps) override;

  void onProcessStart() override;

  void analyze(const framework::Event& event) override;

 private:
  /// EventSummary collection name
  std::string summary_name_;

  /// EventSummary pass name
  std::string summary_pass_;

  /// timestamp [ns] of the previous built event, 0 before the first
  uint64_t last_timestamp_{0};
};

}  // namespace dqm

#endif  // DQM_EVENTSUMMARYDQM_H
