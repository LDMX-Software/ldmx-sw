
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"  //creating unique cell IDs
#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "catch.hpp"  //for TEST_CASE, REQUIRE, and other Catch2 macros

namespace hcal {
namespace test {

/**
 * Maximum percent error that a single hit position
 * can be reconstructed/mapped.
 * 20% for now.
 */
static const double MAX_ENERGY_PERCENT_ERROR_ZPOSITION = 20;
static const double MAX_ENERGY_PERCENT_ERROR_XPOSITION = 20;
static const double MAX_ENERGY_PERCENT_ERROR_YPOSITION = 20;

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

    for (int ihit = 0; ihit < simHits.size(); ihit++) {
      auto hit = simHits.at(ihit);
      double x = hit.getPosition()[0];
      double y = hit.getPosition()[1];
      double z = hit.getPosition()[2];

      unsigned int hitID = hit.getID();
      ldmx::HcalID detID(hitID);
      int section = detID.section();
      int layer = detID.layer();
      int strip = detID.strip();

      // get the Hcal geometry
      const auto &hcalGeometry = getCondition<ldmx::HcalGeometry>(
          ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

      // get position
      auto positionMap = hcalGeometry.getStripCenterPosition(detID);

      auto target_z =
          Approx(z).epsilon(MAX_ENERGY_PERCENT_ERROR_ZPOSITION / 100);
      CHECK(positionMap.Z() == target_z);
      if (section == ldmx::HcalID::HcalSection::BACK) {
        if (layer % 2 == 1) {
          auto target_y =
              Approx(y).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          CHECK(positionMap.Y() == target_y);
        } else {
          auto target_x =
              Approx(x).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          CHECK(positionMap.X() == target_x);
        }
      } else {
        if ((section == ldmx::HcalID::HcalSection::TOP) ||
            (section == ldmx::HcalID::HcalSection::BOTTOM)) {
          auto target_y =
              Approx(y).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          CHECK(positionMap.Y() == target_y);
        } else if ((section == ldmx::HcalID::HcalSection::LEFT) ||
                   (section == ldmx::HcalID::HcalSection::RIGHT)) {
          auto target_x =
              Approx(x).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          CHECK(positionMap.X() == target_x);
        }
      }
    }
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

  char **args;
  framework::ProcessHandle p;

  framework::ConfigurePython cfg(config_file, args, 0);
  REQUIRE_NOTHROW(p = cfg.makeProcess());
  p->run();
}
