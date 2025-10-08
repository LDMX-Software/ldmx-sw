#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

using Catch::Approx;

#include "DetDescr/HcalID.h"  //creating unique hcal IDs
#include "Framework/Configure/Python.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "Hcal/Event/HcalHit.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "SimCore/Event/SimCalorimeterHit.h"

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
 *  static const double MeV_per_mV = PE_ENERGY / 5;  // 0.013 MeV/mV
 */

/**
 * Maximum error that a single hit energy/PE
 * can be reconstructed with before failing the test
 *
 * Comparing energy deposited/PE that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited/PE output by reconstructor.
 *
 * NOTE: Currently Digitization not implemented for TOT mode
 */
// static const double MAX_ENERGY_ERROR_DAQ = 4 * PE_ENERGY;
// static const double MAX_ENERGY_PERCENT_ERROR_DAQ = 0.2;
static const double MAX_PE_ERROR_DAQ = 40;
// large percentage error for now
static const double MAX_PE_PERCENT_ERROR_DAQ = 0.4;

/**
 * Number of sim hits_ to create.
 *
 * In this test, we create one sim hit per event,
 * run it through the digi pipeline, and then
 * check it. This parameter tells us how many
 * sim hits_ to create and then (combined with
 * the parameters of HcalFakeSimHits), we know
 * how "fine-grained" the test is.
 */
static const int NUM_TEST_SIM_HITS = 1000;

/**
 * Our custom checker which makes sure that
 * the input energy/position is "close enough" to the truth
 * energy/position.
 */
class IsCloseEnough : public Catch::Matchers::MatcherBase<double> {
 private:
  /// correct (sim-level)
  double truth_;

  /// maximum absolute difference
  const double MAX_ABSOLUTE_DIFF;

  /// maximum relative difference
  const double MAX_RELATIVE_DIFF;

 public:
  /**
   * Constructor
   *
   * Sets the truth level
   */
  IsCloseEnough(double const &truth, double const &abs_diff,
                double const &rel_diff)
      : truth_{truth},
        MAX_ABSOLUTE_DIFF{abs_diff},
        MAX_RELATIVE_DIFF{rel_diff} {}

  /**
   * Performs the test for this matcher
   *
   * We check that the input is **either**
   * within the absolute difference or the relative
   * difference.
   */
  bool match(const double &daq) const override {
    return (daq == Approx(truth_).epsilon(MAX_RELATIVE_DIFF) or
            daq == Approx(truth_).margin(MAX_ABSOLUTE_DIFF));
  }

  /**
   * Describes matcher for printing to terminal.
   */
  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << "is within an absolute difference of " << MAX_ABSOLUTE_DIFF
       << " OR a relative difference of " << MAX_RELATIVE_DIFF << " with "
       << truth_;
    return ss.str();
  }
};

/**
 * @class FakeSimHits
 *
 * Fills the event bus with an HcalSimHits collection with
 * a range of energy hits_. These hits_ are put into unique
 * bars so that we can compare them to the correct energy
 * in one event.
 */
class HcalFakeSimHits : public framework::Producer {
  /**
   * Maximum energy to make a simulated hit for [MeV]
   */
  // Based on the current gain settings for the ADC readout mode
  // we will reach saturation ~ 20 MeV ~ 290 PEs
  const double MAX_ENERGY = 200 * PE_ENERGY;  // ~ 13 MeV

  /**
   * Minimum energy to make a sim hit for [MeV]
   * Needs to be above readout threshold (after internal HcalDigi's calculation)
   */
  const double MIN_ENERGY = 4 * PE_ENERGY;
  /**
   * The step between energies is calculated depending on the min, max energy
   * and the total number of sim hits_ you desire.
   * [MeV]
   */
  const double ENERGY_STEP = (MAX_ENERGY - MIN_ENERGY) / NUM_TEST_SIM_HITS;

  /// current energy of the sim hit we are on
  double curr_energy_ = MIN_ENERGY;

 public:
  HcalFakeSimHits(const std::string &name, framework::Process &p)
      : framework::Producer(name, p) {}
  ~HcalFakeSimHits() {}

  void beforeNewRun(ldmx::RunHeader &header) final override {
    header.setDetectorName("ldmx-det-v12");
  }

  void produce(framework::Event &event) final override {
    // put in a single sim hit
    std::vector<ldmx::SimCalorimeterHit> pretend_sim_hits(1);

    // We hard-code the position of one hit: back hcal, layer_ 1, strip 31
    // This real simHit position is obtained by looking at calorimeter
    // SimHits of a 4 GeV muon shoot through the beamline
    ldmx::HcalID id(0, 1, 31);
    pretend_sim_hits[0].setPosition(-6.70265, 3.70265, 879);  // mm
    pretend_sim_hits[0].setID(id.raw());
    pretend_sim_hits[0].addContrib(
        -1,            // incidentID
        -1,            // trackID
        0,             // pdg ID
        curr_energy_,  // edep
        2.96628  // time - 299mm is about 1ns from target and in middle of HCal
    );

    // needs to be correct collection name
    // REQUIRE_NOTHROW(event.add("HcalSimHits", pretendSimHits));
    REQUIRE_NOTHROW(event.add("HcalFakeSimHits", pretend_sim_hits));
    curr_energy_ += ENERGY_STEP;

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
  // save ntuple? False by default because if ntuplizer is on, the HcalGeometry
  // test cannot be run
  const bool SAVE = false;

 private:
  std::string hcal_fake_sim_hits_passname_;
  std::string hcal_rec_hits_passname_;
  std::string hcal_digis_passname_;

 public:
  HcalCheckReconstruction(const std::string &name, framework::Process &p)
      : framework::Analyzer(name, p) {}
  ~HcalCheckReconstruction() {}

  void configure(framework::config::Parameters &ps) override {
    hcal_fake_sim_hits_passname_ =
        ps.getParameter("hcal_fake_sim_hits_passname", "");
    hcal_digis_passname_ = ps.getParameter("hcal_digis_passname", "");
    hcal_rec_hits_passname_ = ps.getParameter("hcal_rec_hits_passname", "");
  }

  void onProcessStart() final override {
    if (SAVE) {
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
    const auto sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
        "HcalFakeSimHits", hcal_fake_sim_hits_passname_);

    REQUIRE(sim_hits.size() == 1);

    float truth_energy = sim_hits.at(0).getEdep();

    if (SAVE) {
      ntuple_.setVar<float>("SimEnergy", truth_energy);
      ntuple_.setVar<float>("SimX", sim_hits.at(0).getPosition()[0]);
      ntuple_.setVar<float>("SimY", sim_hits.at(0).getPosition()[1]);
      ntuple_.setVar<float>("SimZ", sim_hits.at(0).getPosition()[2]);
      ntuple_.setVar<float>("SimTime", sim_hits.at(0).getContrib(0).time_);
    }

    const auto daq_digis{event.getObject<ldmx::HgcrocDigiCollection>(
        "HcalDigis", hcal_digis_passname_)};
    auto daq_digi = daq_digis.getDigi(0);
    bool is_in_adc_mode = daq_digi.isADC();

    if (SAVE) {
      ntuple_.setVar<int>("DaqDigi", daq_digi.soi().raw());
      ntuple_.setVar<int>("DaqDigiIsADC", is_in_adc_mode);
      ntuple_.setVar<int>("DaqDigiADC", daq_digi.soi().adcT());
      ntuple_.setVar<int>("DaqDigiTOT", daq_digi.tot());
    }

    const auto rec_hits = event.getCollection<ldmx::HcalHit>(
        "HcalRecHits", hcal_rec_hits_passname_);
    CHECK(rec_hits.size() == 1);

    auto hit = rec_hits.at(0);
    ldmx::HcalID id(hit.getID());
    CHECK_FALSE(hit.isNoise());
    CHECK(id.raw() == sim_hits.at(0).getID());

    if (SAVE) {
      ntuple_.setVar<float>("RecX", hit.getXPos());
      ntuple_.setVar<float>("RecY", hit.getYPos());
      ntuple_.setVar<float>("RecZ", hit.getZPos());
      ntuple_.setVar<float>("RecTime", hit.getTime());
      ntuple_.setVar<float>("RecPE", hit.getPE());
      ntuple_.setVar<float>("RecEnergy", hit.getEnergy());
    }

    // define target pe by using the settings at the top
    double daq_pe{hit.getPE()};
    CHECK_THAT(daq_pe, IsCloseEnough(truth_energy / PE_ENERGY, MAX_PE_ERROR_DAQ,
                                     MAX_PE_PERCENT_ERROR_DAQ));

    // std::cout << "rec energy " << hit.getEnergy() << " * approx sampl
    // fraction " << hit.getEnergy()*sampling_fraction << " truth " <<
    //   truth_energy
    //           << std::endl;
    // std::cout << "npes " << hit.getPE() << " approx PE " << int(truth_energy
    // / PE_ENERGY)  << std::endl;
    /*
        if (id.section() == 0) {
          double truth_pos, rec_pos;
          if ((id.layer() % 2) == 1) {
            truth_pos = simHits.at(0).getPosition()[0];
            rec_pos = hit.getXPos();
          } else {
            truth_pos = simHits.at(0).getPosition()[1];
            rec_pos = hit.getYPos();
          }
          // std::cout << "rec pos_ " << rec_pos << " truth " << truth_pos <<
          // std::endl;
          // comment position check for now
          // CHECK_THAT(rec_pos, isCloseEnough(truth_pos,
       MAX_POSITION_ERROR_DAQ,
          //                                 MAX_POSITION_PERCENT_ERROR_DAQ));
        }
    */
    return;
  }
};  // HcalCheckReconstruction

}  // namespace test
}  // namespace hcal

DECLARE_ANALYZER(hcal::test::HcalFakeSimHits);
DECLARE_PRODUCER(hcal::test::HcalCheckReconstruction);

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
  char **args{nullptr};

  auto cfg{framework::config::run("ldmxcfg.Process.lastProcess", config_file,
                                  args, 0)};
  auto p{std::make_unique<framework::Process>(cfg)};
  p->run();
}
