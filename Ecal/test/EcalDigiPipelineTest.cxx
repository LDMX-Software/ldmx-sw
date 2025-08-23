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
static const double ME_V_PER_F_C = MIP_SI_ENERGY / (37 * 0.1602);

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
  IsCloseEnough(double const &truth, double const &abs_diff,
                double const &rel_diff)
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
  bool match(const double &daq_energy) const override {
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
  const double MAX_ENERGY = 10000. * ME_V_PER_F_C;

  /**
   * Minimum energy to make a sim hit for [MeV]
   * Needs to be above readout threshold (after internal EcalDigi's calculation)
   *
   * One MIP is ~0.13 MeV, so we choose that.
   */
  const double MIN_ENERGY = MIP_SI_ENERGY;

  /**
   * The step between energies is calculated depending on the min, max energy
   * and the total number of sim hits_ you desire.
   * [MeV]
   */
  const double ENERGY_STEP = (MAX_ENERGY - MIN_ENERGY) / NUM_TEST_SIM_HITS;

  /// current energy of the sim hit we are on
  double curr_energy_ = MIN_ENERGY;

 public:
  EcalFakeSimHits(const std::string &name, framework::Process &p)
      : framework::Producer(name, p) {}
  ~EcalFakeSimHits() {}

  void beforeNewRun(ldmx::RunHeader &header) final override {
    header.setDetectorName("ldmx-det-v14-8gev");
  }

  void produce(framework::Event &event) final override {
    // put in a single sim hit
    std::vector<ldmx::SimCalorimeterHit> pretend_sim_hits(1);

    ldmx::EcalID id(0, 0, 0);
    pretend_sim_hits[0].setID(id.raw());
    // incidentID, trackID, pdg ID, edep, time - 299mm is about 1ns from target
    // and in middle of ECal
    pretend_sim_hits[0].addContrib(-1, -1, 0, curr_energy_, 1.);
    // sim position in middle of ECal
    pretend_sim_hits[0].setPosition(0., 0., 299.);

    // needs to be correct collection name
    REQUIRE_NOTHROW(event.add("EcalSimHits", pretend_sim_hits));

    curr_energy_ += ENERGY_STEP;

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

 public:
  EcalCheckEnergyReconstruction(const std::string &name, framework::Process &p)
      : framework::Analyzer(name, p) {}
  ~EcalCheckEnergyReconstruction() {}

  void configure(framework::config::Parameters &parameters) final override {
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

  void analyze(const framework::Event &event) final override {
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

      const auto rec_hits = event.getCollection<ldmx::EcalHit>(
          "EcalRecHits", ecal_rechits_passname_);
      CHECK(rec_hits.size() == 1);

      auto hit = rec_hits.at(0);
      ldmx::EcalID id(hit.getID());
      CHECK_FALSE(hit.isNoise());
      CHECK(id.raw() == sim_hits.at(0).getID());

      double daq_energy{hit.getAmplitude()};
      CHECK_THAT(daq_energy, IsCloseEnough(truth_energy, MAX_ENERGY_ERROR_DAQ,
                                           MAX_ENERGY_PERCENT_ERROR_DAQ));
      ntuple_.setVar<float>("RecEnergy", hit.getAmplitude());

      const auto trig_digis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
          "ecalTrigDigis", ecal_trig_digis_passname_)};
      CHECK(trig_digis.size() == 1);

      auto trig_digi = trig_digis.at(0);
      float tp_energy =
          8 * trig_digi.linearPrimitive() * 320. / 1024 * ME_V_PER_F_C;

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
  char **args{nullptr};

  auto cfg{framework::config::run("ldmxcfg.Process.lastProcess", config_file,
                                  args, 0)};
  auto p{std::make_unique<framework::Process>(cfg)};
  p->run();
}
