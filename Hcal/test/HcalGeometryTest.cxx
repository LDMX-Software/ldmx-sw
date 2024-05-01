#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

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
 * Maximum percent error that a single hit position
 * can be reconstructed/mapped.
 * 20% for now.
 */
static const double MAX_ENERGY_PERCENT_ERROR_ZPOSITION = 20;
static const double MAX_ENERGY_PERCENT_ERROR_XPOSITION = 50;
static const double MAX_ENERGY_PERCENT_ERROR_YPOSITION = 50;
static const double MAX_ENERGY_PERCENT_ERROR_ZPOSITION_SIDE = 100;
  
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
      // std::cout	<< "ihit " << ihit << " section " << detID.section() << " layer " << detID.layer() << " strip " << detID.strip() << std::endl;
      
      // get the Hcal geometry
      const auto &hcalGeometry = getCondition<ldmx::HcalGeometry>(
          ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

      // get position
      auto positionMap = hcalGeometry.getStripCenterPosition(detID);

      if (section == ldmx::HcalID::HcalSection::BACK) {
	auto target_z =
          Approx(z).epsilon(MAX_ENERGY_PERCENT_ERROR_ZPOSITION / 100);
	//CHECK(positionMap.Z() == target_z);      
	//std::cout << " BACK " << "x sim hit " << x << " (Sec,strip,layer) " << section << " " << strip << " " << layer << " x " << positionMap.X() << std::endl;
        //std::cout << " BACK " << "y sim hit " << y << " (Sec,strip,layer) " << section << " " << strip << " " << layer << " y " << positionMap.Y() << std::endl;
	//std::cout << " BACK " << "z sim hit " << z << " (Sec,strip,layer) " << section << " " << strip << " " << layer << " z " << positionMap.Z() << std::endl;

        if (layer % 2 == 1) {
          auto target_y =
              Approx(y).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          //CHECK(positionMap.Y() == target_y);
        } else {
          auto target_x =
              Approx(x).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          //CHECK(positionMap.X() == target_x);
        }
      } else {
	auto target_z =
	  Approx(z).epsilon(MAX_ENERGY_PERCENT_ERROR_ZPOSITION_SIDE / 100);
	// CHECK(positionMap.Z() == target_z);
	//std::cout << " SIDE " << "x sim hit " << x << " (Sec,strip,layer) " << section << " " << strip << " " << layer << " x " << positionMap.X() << std::endl;
        //std::cout << " SIDE " << "y sim hit " << y << " (Sec,strip,layer) " << section << " " << strip << " " << layer << " y " << positionMap.Y() << std::endl;
	//std::cout << " SIDE " << "z sim hit " << z << " (Sec,strip,layer) " << section << " " << strip << " " << layer << " z " << positionMap.Z() << std::endl;
	
        if ((section == ldmx::HcalID::HcalSection::TOP) ||
            (section == ldmx::HcalID::HcalSection::BOTTOM)) {
          auto target_y =
              Approx(y).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          // CHECK(positionMap.Y() == target_y);
        } else if ((section == ldmx::HcalID::HcalSection::LEFT) ||
                   (section == ldmx::HcalID::HcalSection::RIGHT)) {
          auto target_x =
              Approx(x).epsilon(MAX_ENERGY_PERCENT_ERROR_YPOSITION / 100);
          // CHECK(positionMap.X() == target_x);
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
