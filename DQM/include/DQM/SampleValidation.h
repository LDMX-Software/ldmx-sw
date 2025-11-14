#ifndef DQM_SAMPLEVALIDATION_H
#define DQM_SAMPLEVALIDATION_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Math/Vector3D.h"

namespace dqm {

/**
 * @class SampleValidation
 * @brief
 */

class SampleValidation : public framework::Analyzer {
 public:
  SampleValidation(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}
  virtual void configure(framework::config::Parameters& ps) override;
  virtual void analyze(const framework::Event& event) override;
  int pdgidLabel(const int pdgid);

 private:
  std::string target_scoring_plane_passname_;
  std::string sim_particles_passname_;
};
}  // namespace dqm

#endif
