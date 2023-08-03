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
  /*
   *
   * Determine which of x/y/z corresponds to the direction along, across, and
   * through the bar respectively. Along corresponding to the length of the bar,
   * across to the width of the bar, and through to the thickness of the bar.
   *
   *
   */
  std::array<int, 3> determine_indices(const ldmx::HcalID id);

  /*
   *
   * Check if the hit at position `position` is within the bounds of the
   * scintillator bar with HcalID `id`.
   *
   * @note: On error, this function will raise an exception if `stop_on_error`
   * is set to true or log the issue if false.
   *
   */
  bool hit_ok(const ldmx::HcalID id, const std::array<double, 3> &position);

  void analyze(const framework::Event &event) override;

 private:
  std::string hcalSimHitsCollection_{"HcalSimHits"};
  std::string hcalReSimHitsCollection_{"HcalReSimHits"};
  std::string simPassName_{""};
  std::string reSimPassName_{""};

  // Maximum difference between position and bounds of the scintillator bar that
  // will be accepted [mm]
  double tolerance{};
  bool stop_on_error{};
};
}  // namespace dqm

#endif /* RESIMVERIFIER_H */
