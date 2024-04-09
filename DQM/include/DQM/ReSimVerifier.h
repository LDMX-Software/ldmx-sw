#ifndef RESIMVERIFIER_H
#define RESIMVERIFIER_H

#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Hcal/Event/HcalHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace dqm {

class ReSimVerifier : public framework::Analyzer {
 public:
  ReSimVerifier(const std::string &name, framework::Process &process)
      : framework::Analyzer{name, process} {}
  void configure(framework::config::Parameters &parameters) override;

  void analyze(const framework::Event &event) override;

  /*
   * Check that the simhits between the two collections are identical.
   *
   * @return: False if any hit is different between the two
   *
   **/
  bool verifySimCalorimeterHits(
      const std::vector<ldmx::SimCalorimeterHit> &simHits,
      const std::vector<ldmx::SimCalorimeterHit> &reSimHits);

  /*
   * Check that all SimParticles are present in both passes of the event
   *
   * @return: False if any SimParticle is different between the two
   *
   **/
  bool verifySimParticles(const framework::Event &event);

 private:
  std::vector<std::string> collections;
  std::string simPassName_{""};
  std::string reSimPassName_{""};

  /*
   * If true, abort on the first mismatch. Otherwise, report and continue.
   **/
  bool stop_on_error{};
};
}  // namespace dqm

#endif /* RESIMVERIFIER_H */
