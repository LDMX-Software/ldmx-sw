#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "DetDescr/EcalID.h"  //creating unique cell IDs
#include "Ecal/Event/EcalHit.h"
#include "Framework/Configure/Python.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "SimCore/Event/SimCalorimeterHit.h"

using Catch::Approx;

namespace ecal {
namespace test {

/**
 * Energy deposited in our silicon sensors by one MIP on average.
 * [MeV]
 */
static const double MIP_SI_ENERGY = 0.130;

/**
 * Conversion between deposited charge and deposited energy
 * [MeV/fC]
 *
 * charge [fC] * (1000 electrons / 0.1602 fC) * (1 MIP / 37 000 electrons) *
 * (0.130 MeV / 1 MIP)
 */
static const double MEV_PER_FC = MIP_SI_ENERGY / (37 * 0.1602);

/**
 * Maximum percent error that a single hit
 * can be reconstructed with before failing the test
 * if above the tot threshold.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_PERCENT_ERROR_DAQ = 0.025;

/**
 * Maximum percent error that a single hit can be
 * estimated at the trigger primitive level when
 * reading out in TOT mode.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the
 * energy estimated from trigger primitives.
 */
static const double MAX_ENERGY_PERCENT_ERROR_TP = 0.15;

/**
 * Maximum absolute error that a single hit
 * can be reconstructed with before failing the test
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_ERROR_DAQ = MIP_SI_ENERGY / 2;

/**
 * Maximum absolute error that a single hit
 * can be estimated at the trigger primitive level
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the
 * energy estimated from trigger primitives.
 */
static const double MAX_ENERGY_ERROR_TP = 2 * MIP_SI_ENERGY;

/**
 * Number of sim hits_ to create.
 *
 * In this test, we create one sim hit per event,
 * run it through the digi pipeline, and then
 * check it. This parameter tells us how many
 * sim hits_ to create and then (combined with
 * the parameters of EcalFakeSimHits), we know
 * how "fine-grained" the test is.
 */
static const int NUM_TEST_SIM_HITS = 2000;

/**
 * Our custom energy checker which makes sure that
 * the input energy is "close enough" to the truth
 * energy.
 */
class IsCloseEnough : public Catch::Matchers::MatcherBase<double> {
 private:
  /// correct (sim-level) energy [MeV]
  double truth_;

  /// maximum absolute energy difference [MeV]
  const double MAX_ABSOLUTE_DIFF;

  /// maximum relative energy difference
  const double MAX_RELATIVE_DIFF;

 public:
  /**
   * Constructor
   *
   * Sets the truth level energy
   */
  IsCloseEnough(double const& truth, double const& abs_diff,
                double const& rel_diff)
      : truth_{truth},
        MAX_ABSOLUTE_DIFF{abs_diff},
        MAX_RELATIVE_DIFF{rel_diff} {}

  /**
   * Performs the test for this matcher
   *
   * We check that the input energy is **either**
   * within the absolute difference or the relative
   * difference.
   */
  bool match(const double& daq_energy) const override {
    return (daq_energy == Approx(truth_).epsilon(MAX_RELATIVE_DIFF) or
            daq_energy == Approx(truth_).margin(MAX_ABSOLUTE_DIFF));
  }

  /**
   * Describes matcher for printing to terminal.
   */
  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << "is within an absolute difference of " << MAX_ABSOLUTE_DIFF
       << "MeV OR a relative difference of " << MAX_RELATIVE_DIFF << " with "
       << truth_ << " MeV.";
    return ss.str();
  }
};

/**
 * @class FakeSimHits
 *
 * Fills the event bus with an EcalSimHits collection with
 * a range of energy hits_. These hits_ are put into unique
 * cells so that we can compare them to the correct energy
 * in one event.
 */
class EcalFakeSimHits : public framework::Producer {
  /**
   * Maximum energy to make a simulated hit for [MeV]
   *
   * The maximum value to be readout is 4096 TDC which
   * is equivalent to ~10000fC deposited charge.
   */
  double max_energy_;

  /**
   * Minimum energy to make a sim hit for [MeV]
   * Needs to be above readout threshold (after internal EcalDigi's calculation)
   *
   * One MIP is ~0.13 MeV, so we choose that.
   */
  double min_energy_;

  /// last arrival time of the sim hit to make [ns]
  double max_time_;

  /// first arrival time of the sim hit to make [ns]
  double min_time_;

  /**
   * The step between energies (times) is calculated depending on the min, max
   * energy (time) and the total number of sim hits_ you desire.
   */
  double energy_step_;
  double time_step_;

  /// current energy of the sim hit we are on
  double curr_energy_;

  /// current arrival time of the sim hit we are on
  double curr_time_;

 public:
  EcalFakeSimHits(const std::string& name, framework::Process& p)
      : framework::Producer(name, p) {}
  ~EcalFakeSimHits() {}

  void configure(framework::config::Parameters& ps) final override {
    min_energy_ = ps.get<double>("min_energy", MIP_SI_ENERGY);
    max_energy_ = ps.get<double>("max_energy", 10000. * MEV_PER_FC);
    // 299mm is about 1ns from target and in middle of ECal,
    // so the default arrival time of 1ns is an in-time hit
    min_time_ = ps.get<double>("min_time", 1.);
    max_time_ = ps.get<double>("max_time", 1.);

    energy_step_ = (max_energy_ - min_energy_) / NUM_TEST_SIM_HITS;
    time_step_ = (max_time_ - min_time_) / NUM_TEST_SIM_HITS;

    curr_energy_ = min_energy_;
    curr_time_ = min_time_;
  }

  void beforeNewRun(ldmx::RunHeader& header) final override {
    header.setDetectorName("ldmx-det-v15-8gev");
  }

  void produce(framework::Event& event) final override {
    // put in a single sim hit
    std::vector<ldmx::SimCalorimeterHit> pretend_sim_hits(1);

    ldmx::EcalID id(0, 0, 0);
    pretend_sim_hits[0].setID(id.raw());
    // incidentID, trackID, pdg ID, edep, time
    pretend_sim_hits[0].addContrib(-1, -1, 0, curr_energy_, curr_time_);
    // sim position in middle of ECal
    pretend_sim_hits[0].setPosition(0., 0., 299.);

    // needs to be correct collection name
    REQUIRE_NOTHROW(event.add("EcalSimHits", pretend_sim_hits));

    curr_energy_ += energy_step_;
    curr_time_ += time_step_;

    return;
  }
};  // EcalFakeSimHits

/**
 * @class EcalCheckEnergyReconstruction
 *
 * Checks
 * - Amplitude of EcalRecHit matches SimCalorimeterHit EDep with the same ID
 * - Estimated energy at TP level matches sim energy
 *
 * Assumptions
 * - Max one sim hit per event
 * - Noise generation has been turned off
 */
class EcalCheckEnergyReconstruction : public framework::Analyzer {
 private:
  std::string ecal_simhits_passname_;
  std::string ecal_digis_passname_;
  std::string ecal_rechits_passname_;
  std::string ecal_trig_digis_passname_;
  bool check_trig_prim_;

 public:
  EcalCheckEnergyReconstruction(const std::string& name, framework::Process& p)
      : framework::Analyzer(name, p) {}
  ~EcalCheckEnergyReconstruction() {}

  void configure(framework::config::Parameters& parameters) final override {
    // the trigger primitives are an in-time, single-sample estimate,
    // so they are not expected to see out-of-time hits
    check_trig_prim_ = parameters.get<bool>("check_trig_prim", true);
    ecal_simhits_passname_ =
        parameters.get<std::string>("ecal_simhits_passname", "");
    ecal_digis_passname_ =
        parameters.get<std::string>("ecal_digis_passname", "");
    ecal_rechits_passname_ =
        parameters.get<std::string>("ecal_rechits_passname", "");
    ecal_trig_digis_passname_ =
        parameters.get<std::string>("ecal_trig_digis_passname", "");
  }

  void onProcessStart() final override {
    getHistoDirectory();
    ntuple_.create("EcalDigiTest");
    ntuple_.addVar<float>("EcalDigiTest", "SimEnergy");
    ntuple_.addVar<float>("EcalDigiTest", "RecEnergy");
    ntuple_.addVar<float>("EcalDigiTest", "TrigPrimEnergy");

    ntuple_.addVar<int>("EcalDigiTest", "DaqDigi");
    ntuple_.addVar<int>("EcalDigiTest", "DaqDigiIsADC");
    ntuple_.addVar<int>("EcalDigiTest", "DaqDigiADC");
    ntuple_.addVar<int>("EcalDigiTest", "DaqDigiTOT");
    ntuple_.addVar<int>("EcalDigiTest", "TrigPrimDigiEncoded");
    ntuple_.addVar<int>("EcalDigiTest", "TrigPrimDigiLinear");
  }

  void analyze(const framework::Event& event) final override {
    const auto sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
        "EcalSimHits", ecal_simhits_passname_);

    REQUIRE(sim_hits.size() == 1);

    float truth_energy = sim_hits.at(0).getEdep();
    ntuple_.setVar<float>("SimEnergy", truth_energy);

    const auto daq_digis{event.getObject<ldmx::HgcrocDigiCollection>(
        "EcalDigis", ecal_digis_passname_)};

    if (daq_digis.getNumDigis() == 1) {
      auto daq_digi = daq_digis.getDigi(0);
      ntuple_.setVar<int>("DaqDigi", daq_digi.soi().raw());
      bool is_in_adc_mode = daq_digi.isADC();
      ntuple_.setVar<int>("DaqDigiIsADC", is_in_adc_mode);
      ntuple_.setVar<int>("DaqDigiADC", daq_digi.soi().adcT());
      ntuple_.setVar<int>("DaqDigiTOT", daq_digi.tot());

      // arrival time of the hit at the chip, helpful when the failure
      // depends on where in the readout window the pulse lands
      INFO("sim hit arrival time = " << sim_hits.at(0).getContrib(0).time_
                                     << " ns");
      INFO("digi is " << (is_in_adc_mode ? "ADC" : "TOT") << " mode");

      const auto rec_hits = event.getCollection<ldmx::EcalHit>(
          "EcalRecHits", ecal_rechits_passname_);
      CHECK(rec_hits.size() == 1);
      // a hit that was read out but not reconstructed has nothing left
      // for us to check, keep going so that all events are checked
      if (rec_hits.size() != 1) return;

      auto hit = rec_hits.at(0);
      ldmx::EcalID id(hit.getID());
      CHECK_FALSE(hit.isNoise());
      CHECK(id.raw() == sim_hits.at(0).getID());

      double daq_energy{hit.getAmplitude()};
      CHECK_THAT(daq_energy, IsCloseEnough(truth_energy, MAX_ENERGY_ERROR_DAQ,
                                           MAX_ENERGY_PERCENT_ERROR_DAQ));
      ntuple_.setVar<float>("RecEnergy", hit.getAmplitude());

      if (not check_trig_prim_) return;

      const auto trig_digis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
          "ecalTrigDigis", ecal_trig_digis_passname_)};
      CHECK(trig_digis.size() == 1);

      auto trig_digi = trig_digis.at(0);
      float tp_energy =
          8 * trig_digi.linearPrimitive() * 320. / 1024 * MEV_PER_FC;

      CHECK_THAT(tp_energy, IsCloseEnough(truth_energy, MAX_ENERGY_ERROR_TP,
                                          MAX_ENERGY_PERCENT_ERROR_TP));
      ntuple_.setVar<float>("TrigPrimEnergy", tp_energy);
      ntuple_.setVar<int>("TrigPrimDigiEncoded", trig_digi.getPrimitive());
      ntuple_.setVar<int>("TrigPrimDigiLinear", trig_digi.linearPrimitive());
    }

    return;
  }
};  // EcalCheckEnergyReconstruction

}  // namespace test
}  // namespace ecal

DECLARE_PRODUCER(ecal::test::EcalFakeSimHits)
DECLARE_ANALYZER(ecal::test::EcalCheckEnergyReconstruction)

/**
 * Test for the Ecal Digi Pipeline
 *
 * Does not check for realism. Simply makes sure sim energies
 * end up being "close" to output rec energies.
 *
 * Checks
 *  - Keep reconstructed energy depositied close to simulated value
 *  - Keep estimated energy at TP level close to simulated value
 *
 * @TODO still need to expand to multiple contribs in a single sim hit
 * @TODO check layer_ weights are being calculated correctly somehow
 */
TEST_CASE("Ecal Digi Pipeline test", "[Ecal][functionality]") {
  const std::string config_file{"ecal_digi_pipeline_test_config.py"};
  char** args{nullptr};

  auto cfg{framework::config::run("ldmxcfg.Process.last_process", config_file,
                                  args, 0)};
  auto p{std::make_unique<framework::Process>(cfg)};
  p->run();
}

/**
 * Test for the Ecal Digi Pipeline with out-of-time hits
 *
 * A hit large enough to be read out in TOT mode has its TOT measurement
 * reported in the sample where the pulse fell back below threshold, which is
 * only the sample of interest if the hit is in time. This scans the arrival
 * time of such a hit across the readout window and makes sure that, whenever
 * the chip decides to read the hit out, we reconstruct the energy it deposited.
 *
 * @see https://github.com/LDMX-Software/ldmx-sw/issues/1944
 */
TEST_CASE("Ecal Digi Pipeline out-of-time test", "[Ecal][functionality]") {
  const std::string config_file{"ecal_digi_out_of_time_test_config.py"};
  char** args{nullptr};

  auto cfg{framework::config::run("ldmxcfg.Process.last_process", config_file,
                                  args, 0)};
  auto p{std::make_unique<framework::Process>(cfg)};
  p->run();
}
