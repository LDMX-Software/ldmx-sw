#ifndef HCALINEFFICIENCYDQM_H
#define HCALINEFFICIENCYDQM_H
#include <DetDescr/HcalID.h>
#include <Hcal/Event/HcalHit.h>
#include <TCanvas.h>

#include <string>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimCalorimeterHit.h"
namespace dqm {
class HcalInefficiencyAnalyzer : public framework::Analyzer {
 public:
  HcalInefficiencyAnalyzer(const std::string &name, framework::Process &process)
      : framework::Analyzer{name, process} {}

  enum vetoCategories {
    back = 0,
    top = 1,
    bottom = 2,
    right = 3,
    left = 4,
    any = 5,
    both = 6,
    back_only = 7,
    side_only = 8,
    neither = 9
  };
  void configure(framework::config::Parameters &parameters) override;

  bool hitPassesVeto(const ldmx::HcalHit &hit, int section) {
    if (hit.getPE() < pe_veto_threshold || hit.getTime() > max_hit_time_) {
      return true;
    }
    if (section == ldmx::HcalID::HcalSection::BACK && hit.getMinPE() < 1) {
      return true;
    }
    return false;
  }

  void analyze(const framework::Event &event) override;

 private:
  std::string hcalSimHitsCollection_{"HcalSimHits"};
  std::string hcalRecHitsCollection_{"HcalRecHits"};
  std::string hcalSimHitsPassName_{""};
  std::string hcalRecHitsPassName_{""};

  // Veto threshold for photo-electrons
  float pe_veto_threshold;
  double max_hit_time_;
};

}  // namespace dqm

#endif /* HCALINEFFICIENCYDQM_H */
