#ifndef DQM_ECALHITORIGINCLASSIFIERRESULTS_H_
#define DQM_ECALHITORIGINCLASSIFIERRESULTS_H_

#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/** DQM for ECal hit-origin classification performance. */
class EcalHitOriginClassifierResults : public framework::Analyzer {
 public:
  EcalHitOriginClassifierResults(const std::string& name,
                                 framework::Process& process)
      : framework::Analyzer(name, process) {}
  ~EcalHitOriginClassifierResults() override = default;

  void configure(framework::config::Parameters& parameters) override;
  void analyze(const framework::Event& event) override;

 private:
  std::string hit_collection_{"EcalRecHits"};
  std::string hit_pass_name_{};
  std::string classification_collection_{"EcalHitClassifications"};
  std::string classification_pass_name_{};
  std::string sim_hit_collection_{"EcalSimHitsOverlay"};
  std::string sim_hit_pass_name_{"overlay"};
  std::string scoring_plane_collection_{"EcalScoringPlaneHitsOverlay"};
  std::string scoring_plane_pass_name_{"overlay"};
};

}  // namespace dqm

#endif  // DQM_ECALHITORIGINCLASSIFIERRESULTS_H_
