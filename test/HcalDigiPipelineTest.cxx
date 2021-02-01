
#include "catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "DetDescr/HcalID.h" //creating unique cell IDs
#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"

namespace hcal {
namespace test {

/**
 * Energy deposited by one MIP on average
 * [MeV]
 */
static const double MIP_ENERGY = 4.66;

/**
 * Conversion between voltage and deposited energy
 * [MeV/mV]
 */
static const double MeV_per_mV = MIP_ENERGY/(5*68); // 0.013 MeV/mV

/**
 * Maximum percent error that a single hit
 * can be reconstructed with before failing the test
 * if above the tot threshold.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
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
static const double MAX_ENERGY_ERROR_DAQ_ADC_MODE = MIP_ENERGY / 2;

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
static const int NUM_TEST_SIM_HITS = 10;

/**
 * Should the sim/rec/tp energies be ntuplized
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
  const double maxEnergy_  = 3.*MIP_ENERGY; //100

  /**
   * Minimum energy to make a sim hit for [MeV]
   * Needs to be above readout threshold (after internal HcalDigi's calculation)
   */
  //const double minEnergy_ = 1*MIP_ENERGY;
  //const double minEnergy_ = 0.0685*MIP_ENERGY;
  const double minEnergy_ = 0.24*MIP_ENERGY;
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

    // back hcal, layer 8, strip 32 
    ldmx::HcalID id(0,8,32);
    pretendSimHits[0].setPosition( 84.7448, -208.116, 1223.11);
    
    // back hcal, layer 4, strip 30
    //ldmx::HcalID id(0,4,30);
    //pretendSimHits[0].setPosition( -9.90934, 14.4659, 1023.04); 

    // back hcal, layer 5, strip 31
    //ldmx::HcalID id(0,5,31);
    //pretendSimHits[0].setPosition( -16.7397, 42.0896, 1079.69);

    // back hcal, layer 9, strip 33
    //ldmx::HcalID id(0,9,33);                                                                                                                                                                          
    //pretendSimHits[0].setPosition( 0.160346,144.505, 1279.18);
    
    pretendSimHits[0].setID(id.raw());
    pretendSimHits[0].addContrib(-1, //incidentID  
				 -1, // trackID
				 0, // pdg ID
				 currEnergy_, // edep
				 1. // time - 299mm is about 1ns from target and in middle of HCal
    );

    std::cout << "Testing Simhit: x " << pretendSimHits[0].getPosition()[0] << " y " << pretendSimHits[0].getPosition()[1] << " z " << pretendSimHits[0].getPosition()[2];
    std::cout << " with id: section " << id.section() << " layer " << id.layer() << " strip " << id.strip();
    std::cout << " and energy " << currEnergy_ << std::endl;
    
    // needs to be correct collection name
    REQUIRE_NOTHROW(event.add("HcalSimHits", pretendSimHits));
    std::cout << "here "<< std::endl;
    currEnergy_ += energyStep_;

    return;
  }
}; // HcalFakeSimHits

/**
 * @class HcalCheckEnergyReconstruction
 *
 * Checks
 * - Energy of HcalRecHit matches SimCalorimeterHit EDep with the same ID
 * - Estimated energy at TP level matches sim energy
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

    std::cout << "get ampl " << hit.getEnergy() << " taget daq " << truth_energy << " + " << MAX_ENERGY_ERROR_DAQ_ADC_MODE << std::endl;
    CHECK(hit.getEnergy() == target_daq_energy);
    ntuple_.setVar<float>("RecEnergy", hit.getEnergy());
    
    std::cout << "get x " << hit.getXPos() << " y " << hit.getYPos() << " z " << hit.getZPos() << std::endl;
    
    return;
  }
}; // HcalCheckEnergyReconstruction

} // namespace test
} // namespace hcal

DECLARE_PRODUCER_NS(hcal::test, HcalFakeSimHits)
DECLARE_ANALYZER_NS(hcal::test, HcalCheckEnergyReconstruction)

/**
 * Test for the Hcal Digi Pipeline
 *
 * Does not check for realism. Simply makes sure sim energies
 * end up being "close" to output rec energies.
 *
 * Checks
 *  - Keep reconstructed energy depositied close to simulated value
 *  - Keep estimated energy at TP level close to simulated value
 *
 * @TODO still need to expand to multiple contribs in a single sim hit
 * @TODO check layer weights are being calculated correctly somehow
 */
TEST_CASE("Hcal Digi Pipeline test", "[Hcal][functionality]") {

  const std::string config_file{"hcal_digi_pipeline_test_config.py"};

  char **args;
  framework::ProcessHandle p;

  framework::ConfigurePython cfg(config_file, args, 0);
  REQUIRE_NOTHROW(p = cfg.makeProcess());
  p->run();
}
