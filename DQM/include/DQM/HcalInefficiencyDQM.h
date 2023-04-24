#ifndef HCALINEFFICIENCYDQM_H
#define HCALINEFFICIENCYDQM_H
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include <DetDescr/HcalID.h>
#include <Hcal/Event/HcalHit.h>
#include <string>
namespace dqm {
class HcalInefficiencyAnalyzer : public framework::Analyzer {
public:
  HcalInefficiencyAnalyzer(const std::string &name, framework::Process &process)
      : framework::Analyzer{name, process} {}

  void configure(framework::config::Parameters &parameters) {}
  bool hitPassesVeto(const ldmx::HcalHit &hit) {
    return hit.getPE() > 0 && hit.getMinPE() > 0;
  }
  void analyze(const framework::Event &event) override {
    const auto hcalSimHits = event.getCollection<ldmx::SimCalorimeterHit>(
        hcalSimHitsCollection_, hcalSimHitsPassName_);
    const auto hcalRecHits = event.getCollection<ldmx::HcalHit>(
        hcalRecHitsCollection_, hcalRecHitsPassName_);

    /* const auto hcalSimHits =
     * event.getCollection<ldmx::SimCalorimeterHit>(hcalSimHitsCollection_,
     * hcalSimHitsPassName_); */

    /* int firstLayerHit {9999}; */
    double firstHitZ{5500};

    for (const auto hit : hcalRecHits) {
      const ldmx::HcalID id{hit.getID()};
      if (id.section() != ldmx::HcalID::HcalSection::BACK) {
        continue;
      }
      const auto z{hit.getZPos()};

      if (hitPassesVeto(hit)) {
        /* const auto& [x,y,z] {hit.getPosition()}; */
        if (z < firstHitZ) {
          firstHitZ = z;
        }
      }
    }
    histograms_.fill("Inefficiency", firstHitZ);
  }

  std::string hcalSimHitsCollection_{"HcalSimHits"};
  std::string hcalRecHitsCollection_{"HcalRecHits"};
  std::string hcalSimHitsPassName_{""};
  std::string hcalRecHitsPassName_{""};
};

} // namespace dqm

#endif /* HCALINEFFICIENCYDQM_H */
