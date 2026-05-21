#ifndef DQM_CASCADEHISTORYDQM_H
#define DQM_CASCADEHISTORYDQM_H

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/CascadeHistory.h"
#include "SimCore/Event/CascadeStep.h"

namespace dqm {

/**
 * @class CascadeHistoryDQM
 * @brief DQM analyzer for Bertini cascade history data
 *
 * Histograms cascade step counts, particle types, generations, energies,
 * interaction/escape rates, and de-excitation products from
 * BertiniWithHistoryModel.
 */
class CascadeHistoryDQM : public framework::Analyzer {
 public:
  CascadeHistoryDQM(const std::string& name, framework::Process& process);
  virtual ~CascadeHistoryDQM() = default;

  void configure(framework::config::Parameters& parameters) override;
  void analyze(const framework::Event& event) override;

 private:
  void analyzeCascade(const ldmx::CascadeHistory& history);

  /**
   * Get particle category for histogram binning
   * Returns: 0=proton, 1=neutron, 2=pi+, 3=pi-, 4=pi0, 5=kaon, 6=other
   */
  int getParticleCategory(int pdgId) const;

  std::string cascade_coll_name_;
  std::string cascade_pass_name_;
};

}  // namespace dqm

#endif  // DQM_CASCADEHISTORYDQM_H
