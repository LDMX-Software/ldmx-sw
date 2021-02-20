
#include "catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "DetDescr/HcalID.h" //creating unique cell IDs
#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"

namespace hcal {
namespace test {

/**
 * Energy deposited by one photo-electron(PE) on average
 * [MeV]
 * 1 MIP deposits ~ 4.66 MeV
 * 1 MIP ~ 68 PEs
 */
static const double PE_ENERGY = 4.66/68; // 0.069 MeV

/**
 * Conversion between voltage and deposited energy
 * [MeV/mV]
 * 1 PE ~ 5 mV
 */
static const double MeV_per_mV = PE_ENERGY/5; // 0.013 MeV/mV

/**
 * Maximum percent error that a single hit
 * can be reconstructed with before failing the test
 * if above the tot threshold.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 * 
 * NOTE: Currently not implemented for TOT mode so this will not be tested.
 */
static const double MAX_ENERGY_PERCENT_ERROR_DAQ_TOT_MODE = 2.;

/**
 * Maximum absolute error that a single hit
 * can be reconstructed with before failing the test
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_ERROR_DAQ_ADC_MODE = 4.66 / 2; // MIP_ENERGY/2

/**
 * Number of sim hits to create.
 *
 * In this test, we create one sim hit per event,
 * run it through the digi pipeline, and then
 * check it. This parameter tells us how many
 * sim hits to create and then (combined with
 * the parameters of HcalFakeSimHits), we know
 * how "fine-grained" the test is.
 */
static const int NUM_TEST_SIM_HITS = 100;

/**
 * Should the sim/rec energies be ntuplized
 * for your viewing?
 */
static const bool NTUPLIZE_ENERGIES = true;

/**
 * @class FakeSimHits
 *
 * Fills the event bus with an HcalSimHits collection with
 * a range of energy hits. These hits are put into unique
 * cells so that we can compare them to the correct energy
 * in one event.
 */
class HcalFakeSimHits : public framework::Producer {

  /**
   * Maximum energy to make a simulated hit for [MeV]
   */
  const double maxEnergy_  = 300*PE_ENERGY;

  /**
   * Minimum energy to make a sim hit for [MeV]
   * Needs to be above readout threshold (after internal HcalDigi's calculation)
   */
  const double minEnergy_ = 20*PE_ENERGY;
  /**
   * The step between energies is calculated depending on the min, max energy
   * and the total number of sim hits you desire.
   * [MeV]
   */
  const double energyStep_ = (maxEnergy_ - minEnergy_) / NUM_TEST_SIM_HITS;

  /// current energy of the sim hit we are on
  double currEnergy_ = minEnergy_;
  
public:
  HcalFakeSimHits(const std::string &name, framework::Process &p)
    : framework::Producer(name, p) {}
  ~HcalFakeSimHits() {}

  void beforeNewRun(ldmx::RunHeader &header) {
    header.setDetectorName("ldmx-det-v12");
  }

  void produce(framework::Event &event) final override {
    // put in a single sim hit
    std::vector<ldmx::SimCalorimeterHit> pretendSimHits(1);

    // We hard-code the position of one hit: back hcal, layer 8, strip 32
    // This real simHit position is obtained by looking at calorimeter
    // SimHits of a 4 GeV muon shoot through the Hcal
    ldmx::HcalID id(0,8,32);
    pretendSimHits[0].setPosition( 84.7448, -208.116, 1223.11); 
    pretendSimHits[0].setID(id.raw());
    pretendSimHits[0].addContrib(-1, //incidentID  
				 -1, // trackID
				 0, // pdg ID
				 currEnergy_, // edep
				 1. // time - 299mm is about 1ns from target and in middle of HCal
    );

    // needs to be correct collection name
    REQUIRE_NOTHROW(event.add("HcalSimHits", pretendSimHits));
    currEnergy_ += energyStep_;

    return;
  }
}; // HcalFakeSimHits

/**
 * @class HcalCheckEnergyReconstruction
 *
 * Checks
 * - Energy of HcalRecHit matches SimCalorimeterHit EDep with the same ID
 *
 * Assumptions
 * - Only one sim hit per event
 * - Noise generation has been turned off
 */
class HcalCheckEnergyReconstruction : public framework::Analyzer {

public:
  HcalCheckEnergyReconstruction(const std::string &name, framework::Process &p)
      : framework::Analyzer(name, p) {}
  ~HcalCheckEnergyReconstruction() {}

  void onProcessStart() final override {

    getHistoDirectory();
    ntuple_.create("HcalDigiTest");
    ntuple_.addVar<float>("HcalDigiTest", "SimEnergy");
    ntuple_.addVar<float>("HcalDigiTest", "RecEnergy");
    ntuple_.addVar<float>("HcalDigiTest","SimX");
    ntuple_.addVar<float>("HcalDigiTest","SimY");
    ntuple_.addVar<float>("HcalDigiTest","SimZ");
    ntuple_.addVar<float>("HcalDigiTest","RecX");
    ntuple_.addVar<float>("HcalDigiTest","RecY");
    ntuple_.addVar<float>("HcalDigiTest","RecZ");
    
    ntuple_.addVar<int>("HcalDigiTest", "DaqDigi");
    ntuple_.addVar<int>("HcalDigiTest", "DaqDigiIsADC");
    ntuple_.addVar<int>("HcalDigiTest", "DaqDigiADC");
    ntuple_.addVar<int>("HcalDigiTest", "DaqDigiTOT");
  }

  void analyze(const framework::Event &event) final override {

    const auto simHits = event.getCollection<ldmx::SimCalorimeterHit>("HcalSimHits");

    REQUIRE(simHits.size() == 1);

    float truth_energy = simHits.at(0).getEdep();
    ntuple_.setVar<float>("SimEnergy", truth_energy);
    ntuple_.setVar<float>("SimX", simHits.at(0).getPosition()[0]);
    ntuple_.setVar<float>("SimY", simHits.at(0).getPosition()[1]);
    ntuple_.setVar<float>("SimZ", simHits.at(0).getPosition()[2]);
    
    const auto daqDigis{
      event.getObject<ldmx::HgcrocDigiCollection>("HcalDigis")};
    auto daqDigi = daqDigis.getDigi(0);
    ntuple_.setVar<int>("DaqDigi",daqDigi.soi().raw());
    bool is_in_adc_mode = daqDigi.isADC();
    ntuple_.setVar<int>("DaqDigiIsADC",is_in_adc_mode);
    ntuple_.setVar<int>("DaqDigiADC",daqDigi.soi().adc_t());
    ntuple_.setVar<int>("DaqDigiTOT",daqDigi.tot());

    const auto recHits = event.getCollection<ldmx::HcalHit>("HcalRecHits");
    CHECK(recHits.size() == 1);

    auto hit = recHits.at(0);
    ldmx::HcalID id(hit.getID());
    CHECK_FALSE(hit.isNoise());
    CHECK(id.raw() == simHits.at(0).getID());

    ntuple_.setVar<float>("RecX", hit.getXPos());
    ntuple_.setVar<float>("RecY", hit.getYPos());
    ntuple_.setVar<float>("RecZ", hit.getZPos());
    
    // define target energy by using the settings at the top
    auto target_daq_energy = Approx(truth_energy).epsilon(MAX_ENERGY_PERCENT_ERROR_DAQ_TOT_MODE/100);
    if (is_in_adc_mode) target_daq_energy = Approx(truth_energy).margin(MAX_ENERGY_ERROR_DAQ_ADC_MODE);

    CHECK(hit.getEnergy() == target_daq_energy);
    ntuple_.setVar<float>("RecEnergy", hit.getEnergy());
        
    return;
  }
}; // HcalCheckEnergyReconstruction

/**
 * @class HcalCheckPositionReconstruction
 * Checks:
 * - Position of HcalRecHit matches SimCalorimeterHit with the same ID 
 * The SimHits are all generated at the same energy (1 MIP) for consistency.
 */
class HcalCheckPositionReconstruction : public framework::Analyzer {

public:
  HcalCheckPositionReconstruction(const std::string &name, framework::Process &p)
      : framework::Analyzer(name, p) {}
  ~HcalCheckPositionReconstruction() {}

  void onProcessStart() final override {
  }

  void analyze(const framework::Event &event) final override {
    const auto simHits = event.getCollection<ldmx::SimCalorimeterHit>("HcalSimHits");

    CHECK(simHits.size() > 0);

    for(int ihit=0; ihit<simHits.size(); ihit++) {
      auto hit = simHits.at(ihit);
      double x = hit.getPosition()[0];
      double y = hit.getPosition()[1];
      double z = hit.getPosition()[2];
    }

    return;
  }
}; // HcalCheckPositionReconstruction 
  
} // namespace test
} // namespace hcal

DECLARE_PRODUCER_NS(hcal::test, HcalFakeSimHits)
DECLARE_ANALYZER_NS(hcal::test, HcalCheckEnergyReconstruction)
DECLARE_ANALYZER_NS(hcal::test, HcalCheckPositionReconstruction)

/**
 * Test for the Hcal Digi Pipeline
 *
 * Does not check for realism. Simply makes sure sim energies
 * end up being "close" to output rec energies.
 *
 * Checks
 *  - Keep reconstructed energy deposited close to simulated value (with ADC readout mode)
 *  - Keep reconstructed positions close to simulated or expected value
 *
 * @TODO still need to expand to multiple contribs in a single sim hit
 * @TODO check with TOT mode when implemented
 */
TEST_CASE("Hcal Digi Pipeline test", "[Hcal][functionality]") {

  const std::string config_file{"hcal_digi_pipeline_test_config.py"};

  char **args;
  framework::ProcessHandle p;

  framework::ConfigurePython cfg(config_file, args, 0);
  REQUIRE_NOTHROW(p = cfg.makeProcess());
  p->run();
}
TEST_CASE("Hcal Geometry test", "[Hcal][functionality]") {

  const std::string config_file{"hcal_geometry_test_config.py"};

  char **args;
  framework::ProcessHandle p;

  framework::ConfigurePython cfg(config_file, args, 0);
  REQUIRE_NOTHROW(p = cfg.makeProcess());
  //p->run();
}
