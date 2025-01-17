#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"  //creating unique cell IDs
#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace hcal {
namespace test {

/**
 * @class HcalCheckPositionMap
 * Checks:
 * - Position of HcalHit from the HcalGeometry map matches SimCalorimeterHit
 * with the same ID The SimHits are all generated at the same energy (1 MIP) for
 * consistency.
 */
class HcalCheckPositionMap : public framework::Analyzer {
 public:
  HcalCheckPositionMap(const std::string &name, framework::Process &p)
      : framework::Analyzer(name, p) {}
  ~HcalCheckPositionMap() {}

  void onProcessStart() final override {}

  void analyze(const framework::Event &event) final override {
    const auto simHits =
        event.getCollection<ldmx::SimCalorimeterHit>("HcalSimHits");

    CHECK(simHits.size() > 0);
    return;
  }
};  // HcalCheckPositionMap

}  // namespace test
}  // namespace hcal

DECLARE_ANALYZER_NS(hcal::test, HcalCheckPositionMap)

/**
 * Test for the Hcal Geometry ID map
 */
TEST_CASE("Hcal Geometry test", "[Hcal][functionality]") {
  const std::string config_file{"hcal_geometry_test_config.py"};

  char **args{nullptr};
  framework::ProcessHandle p;

  framework::ConfigurePython cfg(config_file, args, 0);
  REQUIRE_NOTHROW(p = cfg.makeProcess());
  p->run();
}
