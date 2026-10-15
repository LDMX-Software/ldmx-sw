#ifndef DQM_PARTICLEFLOWDQM_H
#define DQM_PARTICLEFLOWDQM_H

#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * @class ParticleFlowDQM
 * @brief Monitor PF candidates and charged truth residuals
 */
class ParticleFlowDQM : public framework::Analyzer {
 public:
  ParticleFlowDQM(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}
  ~ParticleFlowDQM() override = default;

  void configure(framework::config::Parameters& ps) override;
  void analyze(const framework::Event& event) override;

 private:
  std::string pf_cand_coll_name_{"PFCandidates"};
  std::string pf_cand_pass_name_{""};
};

}  // namespace dqm

#endif
