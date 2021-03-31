
#include "DetDescr/HcalID.h"  //creating unique hcal IDs
#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "catch.hpp"  //for TEST_CASE, REQUIRE, and other Catch2 macros

namespace hcal {
namespace test {

/**
 * Energy deposited by one photo-electron(PE) on average
 * [MeV]
 * 1 MIP deposits ~ 4.66 MeV
 * 1 MIP ~ 68 PEs
 */
static const double PE_ENERGY = 4.66 / 68;  // 0.069 MeV

/**
 * Conversion between voltage and deposited energy
 * [MeV/mV]
 * 1 PE ~ 5 mV
 */
static const double MeV_per_mV = PE_ENERGY / 5;  // 0.013 MeV/mV

/**
 * Maximum error that a single hit energy
 * can be reconstructed with before failing the test
 *
 * Comparing energy deposited that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 *
 * NOTE: Currently Digitization not implemented for TOT mode
 */
static const double MAX_ENERGY_ERROR_DAQ = 4 * PE_ENERGY;
static const double MAX_ENERGY_PERCENT_ERROR_DAQ = 0.12;

/**
 * Maximum error that a single hit position along the bar
 * can be reconstructed with before failing the test
 * if in the back Hcal
 *
 * Comparing simulated position vs
 * reconstructed position along the bar for even/odd layers in the back Hcal.
 */
static const double MAX_POSITION_ERROR_DAQ =
    50. / 2;  // mm // scintillator length/2
static const double MAX_POSITION_PERCENT_ERROR_DAQ = 0.3;

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
static const int NUM_TEST_SIM_HITS = 1000;

/**
 * Our custom checker which makes sure that
 * the input energy/position is "close enough" to the truth
 * energy/position.
 */
class isCloseEnough : public Catch::MatcherBase<double> {
 private:
  /// correct (sim-level) 
  double truth_;

  /// maximum absolute difference 
  const double max_absolute_diff_;

  /// maximum relative difference
  const double max_relative_diff_;

 public:
  /**
   * Constructor
   *
   * Sets the truth level 
   */
  isCloseEnough(double const &truth, double const &abs_diff,
                double const &rel_diff)
      : truth_{truth},
        max_absolute_diff_{abs_diff},
        max_relative_diff_{rel_diff} {}

  /**
   * Performs the test for this matcher
   *
   * We check that the input is **either**
   * within the absolute difference or the relative
   * difference.
   */
  bool match(const double &daq) const override {
    return (daq == Approx(truth_).epsilon(max_relative_diff_) or
            daq == Approx(truth_).margin(max_absolute_diff_));
  }

  /**
   * Describes matcher for printing to terminal.
   */
  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << "is within an absolute difference of " << max_absolute_diff_
       << " OR a relative difference of " << max_relative_diff_ << " with "
       << truth_ ;
    return ss.str();
  }
};

/**
 * @class FakeSimHits
 *
 * Fills the event bus with an HcalSimHits collection with
 * a range of energy hits. These hits are put into unique
 * bars so that we can compare them to the correct energy
 * in one event.
 */
class HcalFakeSimHits : public framework::Producer {
  /**
   * Maximum energy to make a simulated hit for [MeV]
   */
  // Based on the current gain settings for the ADC readout mode
  // we will reach saturation ~ 20 MeV ~ 290 PEs
  const double maxEnergy_ = 200 * PE_ENERGY; // ~ 13 MeV
  
  /**
   * Minimum energy to make a sim hit for [MeV]
   * Needs to be above readout threshold (after internal HcalDigi's calculation)
   */
  const double minEnergy_ = 4 * PE_ENERGY;
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

    // We hard-code the position of one hit: back hcal, layer 1, strip 31
    // This real simHit position is obtained by looking at calorimeter
    // SimHits of a 4 GeV muon shoot through the beamline
    ldmx::HcalID id(0, 1, 31);
    pretendSimHits[0].setPosition(-6.70265, 3.70265, 879);  // mm
    pretendSimHits[0].setID(id.raw());
    pretendSimHits[0].addContrib(
        -1,           // incidentID
        -1,           // trackID
        0,            // pdg ID
        currEnergy_,  // edep
        2.96628  // time - 299mm is about 1ns from target and in middle of HCal
    );

    // needs to be correct collection name
    // REQUIRE_NOTHROW(event.add("HcalSimHits", pretendSimHits));
    REQUIRE_NOTHROW(event.add("HcalFakeSimHits", pretendSimHits));
    currEnergy_ += energyStep_;

    return;
  }
};  // HcalFakeSimHits

/**
 * @class HcalCheckReconstruction
 *
 * Checks
 * - Energy of HcalRecHit matches SimCalorimeterHit EDep with the same ID
 * - Position of HcalRecHit for back Hcal matches SimCalorimeterHit position
 * along the bar with the same ID
 *
 * Assumptions
 * - Only one sim hit per event
 * - Noise generation has been turned off
 */
class HcalCheckReconstruction : public framework::Analyzer {
  const bool save_ = true;
  
 public:
  HcalCheckReconstruction(const std::string &name, framework::Process &p)
      : framework::Analyzer(name, p) {}
  ~HcalCheckReconstruction() {}

  void onProcessStart() final override {
    if(save_){
      getHistoDirectory();
      ntuple_.create("HcalDigiTest");
      ntuple_.addVar<float>("HcalDigiTest", "SimEnergy");
      ntuple_.addVar<float>("HcalDigiTest", "RecEnergy");
      ntuple_.addVar<float>("HcalDigiTest", "SimX");
      ntuple_.addVar<float>("HcalDigiTest", "SimY");
      ntuple_.addVar<float>("HcalDigiTest", "SimZ");
      ntuple_.addVar<float>("HcalDigiTest", "SimTime");
      ntuple_.addVar<float>("HcalDigiTest", "RecX");
      ntuple_.addVar<float>("HcalDigiTest", "RecY");
      ntuple_.addVar<float>("HcalDigiTest", "RecZ");
      ntuple_.addVar<float>("HcalDigiTest", "RecTime");
      
      ntuple_.addVar<int>("HcalDigiTest", "DaqDigi");
      ntuple_.addVar<int>("HcalDigiTest", "DaqDigiIsADC");
      ntuple_.addVar<int>("HcalDigiTest", "DaqDigiADC");
      ntuple_.addVar<int>("HcalDigiTest", "DaqDigiTOT");
    }
  }


  void analyze(const framework::Event &event) final override {
    const auto simHits =
        event.getCollection<ldmx::SimCalorimeterHit>("HcalFakeSimHits");

    REQUIRE(simHits.size() == 1);

    float truth_energy = simHits.at(0).getEdep();

    if(save_){
      ntuple_.setVar<float>("SimEnergy", truth_energy);
      ntuple_.setVar<float>("SimX", simHits.at(0).getPosition()[0]);
      ntuple_.setVar<float>("SimY", simHits.at(0).getPosition()[1]);
      ntuple_.setVar<float>("SimZ", simHits.at(0).getPosition()[2]);
      ntuple_.setVar<float>("SimTime", simHits.at(0).getContrib(0).time);
    }
    
    const auto daqDigis{
        event.getObject<ldmx::HgcrocDigiCollection>("HcalDigis")};
    auto daqDigi = daqDigis.getDigi(0);
    bool is_in_adc_mode = daqDigi.isADC();

    if(save_){
      ntuple_.setVar<int>("DaqDigi", daqDigi.soi().raw());
      ntuple_.setVar<int>("DaqDigiIsADC", is_in_adc_mode);
      ntuple_.setVar<int>("DaqDigiADC", daqDigi.soi().adc_t());
      ntuple_.setVar<int>("DaqDigiTOT", daqDigi.tot());
    }
    
    const auto recHits = event.getCollection<ldmx::HcalHit>("HcalRecHits");
    CHECK(recHits.size() == 1);

    auto hit = recHits.at(0);
    ldmx::HcalID id(hit.getID());
    CHECK_FALSE(hit.isNoise());
    CHECK(id.raw() == simHits.at(0).getID());

    if(save_){
      ntuple_.setVar<float>("RecX", hit.getXPos());
      ntuple_.setVar<float>("RecY", hit.getYPos());
      ntuple_.setVar<float>("RecZ", hit.getZPos());
      ntuple_.setVar<float>("RecTime", hit.getTime());
      ntuple_.setVar<float>("RecEnergy", hit.getEnergy());
    }
    
    // define target energy by using the settings at the top
    double daq_energy{hit.getEnergy()};
    CHECK_THAT(daq_energy, isCloseEnough(truth_energy, MAX_ENERGY_ERROR_DAQ,
                                         MAX_ENERGY_PERCENT_ERROR_DAQ));

    std::cout << "rec energy " << hit.getEnergy() << " truth " <<
      truth_energy << std::endl;

    // ntuple_.setVar<float>("RecEnergy", hit.getEnergy());

    if (id.section() == 0) {
      double truth_pos, rec_pos;
      if ((id.layer() % 2) == 1) {
        truth_pos = simHits.at(0).getPosition()[0];
        rec_pos = hit.getXPos();
      } else {
        truth_pos = simHits.at(0).getPosition()[1];
        rec_pos = hit.getYPos();
      }
      std::cout << "rec pos " << rec_pos << " truth " << truth_pos << std::endl;
      CHECK_THAT(rec_pos, isCloseEnough(truth_pos, MAX_POSITION_ERROR_DAQ,
                                        MAX_POSITION_PERCENT_ERROR_DAQ));
    }

    return;
  }
};  // HcalCheckReconstruction

}  // namespace test
}  // namespace hcal

DECLARE_PRODUCER_NS(hcal::test, HcalFakeSimHits)
DECLARE_ANALYZER_NS(hcal::test, HcalCheckReconstruction)

/**
 * Test for the Hcal Digi Pipeline
 *
 * Does not check for realism. Simply makes sure sim energies
 * end up being "close" to output rec energies.
 *
 * Checks
 *  - Keep reconstructed energy deposited close to simulated value (with ADC
 * readout mode)
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
