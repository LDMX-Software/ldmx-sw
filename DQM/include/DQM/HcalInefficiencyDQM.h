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

    double firstHitZ{9999};
    double secondHitZ{9999};
    double firstHitLayer{200};
    double secondHitLayer{200};

    for (const auto hit : hcalRecHits) {
      const ldmx::HcalID id{hit.getID()};
      if (id.section() != ldmx::HcalID::HcalSection::BACK) {
        continue;
      }
      const auto z{hit.getZPos()};
      const auto layer{id.layer()};
      if (hitPassesVeto(hit)) {
        if (z < firstHitZ) {
          firstHitZ = z;
          secondHitZ = firstHitZ;
        } else if (z < secondHitZ) {
          secondHitZ = z;
        }
        if (layer < firstHitLayer) {
          secondHitLayer = firstHitLayer;
          firstHitLayer = layer;
        } else if (layer < secondHitLayer && layer != firstHitLayer) {
          secondHitLayer = layer;
        }
      }
    }
    histograms_.fill("Inefficiency", firstHitZ);
    histograms_.fill("TwoHitInefficiency", secondHitZ);
    histograms_.fill("InefficiencyLayer", firstHitLayer);
    histograms_.fill("TwoHitInefficiencyLayer", secondHitLayer);
  }

  std::string hcalSimHitsCollection_{"HcalSimHits"};
  std::string hcalRecHitsCollection_{"HcalRecHits"};
  std::string hcalSimHitsPassName_{""};
  std::string hcalRecHitsPassName_{""};
};

} // namespace dqm

#endif /* HCALINEFFICIENCYDQM_H */
