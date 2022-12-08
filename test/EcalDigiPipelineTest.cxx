#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/catch_approx.hpp>

#include "DetDescr/EcalID.h"  //creating unique cell IDs
#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"

#include "SimCore/Event/SimCalorimeterHit.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Ecal/Event/EcalHit.h"

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
static const double MeV_per_fC = MIP_SI_ENERGY / (37 * 0.1602);

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
 * Number of sim hits to create.
 *
 * In this test, we create one sim hit per event,
 * run it through the digi pipeline, and then
 * check it. This parameter tells us how many
 * sim hits to create and then (combined with
 * the parameters of EcalFakeSimHits), we know
 * how "fine-grained" the test is.
 */
static const int NUM_TEST_SIM_HITS = 2000;

/**
 * Should the sim/rec/tp energies be ntuplized
 * for your viewing?
 */
static const bool NTUPLIZE_ENERGIES = true;

/**
 * Our custom energy checker which makes sure that
 * the input energy is "close enough" to the truth
 * energy.
 */
class isCloseEnough : public Catch::Matchers::MatcherBase<double> {
 private:
  /// correct (sim-level) energy [MeV]
  double truth_;

  /// maximum absolute energy difference [MeV]
  const double max_absolute_diff_;

  /// maximum relative energy difference
  const double max_relative_diff_;

 public:
  /**
   * Constructor
   *
   * Sets the truth level energy
   */
  isCloseEnough(double const &truth, double const &abs_diff,
                double const &rel_diff)
      : truth_{truth},
        max_absolute_diff_{abs_diff},
        max_relative_diff_{rel_diff} {}

  /**
   * Performs the test for this matcher
   * 
   * We check that the input energy is **either**
   * within the absolute difference or the relative
   * difference.
   */
  bool match(const double& daq_energy) const override {
    return (
        daq_energy == Approx(truth_).epsilon(max_relative_diff_)
        or
        daq_energy == Approx(truth_).margin(max_absolute_diff_)
        );
  }

  /**
   * Describes matcher for printing to terminal.
   */
  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << "is within an absolute difference of "
      << max_absolute_diff_ << "MeV OR a relative difference of "
      << max_relative_diff_ << " with " << truth_ << " MeV.";
    return ss.str();
  }
};

/**
 * @class FakeSimHits
 *
 * Fills the event bus with an EcalSimHits collection with
 * a range of energy hits. These hits are put into unique
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
  const double maxEnergy_ = 10000. * MeV_per_fC;

  /**
   * Minimum energy to make a sim hit for [MeV]
   * Needs to be above readout threshold (after internal EcalDigi's calculation)
   *
   * One MIP is ~0.13 MeV, so we choose that.
   */
  const double minEnergy_ = MIP_SI_ENERGY;

  /**
   * The step between energies is calculated depending on the min, max energy
   * and the total number of sim hits you desire.
   * [MeV]
   */
  const double energyStep_ = (maxEnergy_ - minEnergy_) / NUM_TEST_SIM_HITS;

  /// current energy of the sim hit we are on
  double currEnergy_ = minEnergy_;

 public:
  EcalFakeSimHits(const std::string &name, framework::Process &p)
      : framework::Producer(name, p) {}
  ~EcalFakeSimHits() {}

  void beforeNewRun(ldmx::RunHeader &header) {
    header.setDetectorName("ldmx-det-v12");
  }

  void produce(framework::Event &event) final override {
    // put in a single sim hit
    std::vector<ldmx::SimCalorimeterHit> pretendSimHits(1);

    ldmx::EcalID id(0, 0, 0);
    pretendSimHits[0].setID(id.raw());
    pretendSimHits[0].addContrib(
        -1  // incidentID
        ,
        -1  // trackID
        ,
        0  // pdg ID
        ,
        currEnergy_  // edep
        ,
        1.  // time - 299mm is about 1ns from target and in middle of ECal
    );
    pretendSimHits[0].setPosition(0., 0.,
                                  299.);  // sim position in middle of ECal

    // needs to be correct collection name
    REQUIRE_NOTHROW(event.add("EcalSimHits", pretendSimHits));

    currEnergy_ += energyStep_;

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
 * - Only one sim hit per event
 * - Noise generation has been turned off
 */
class EcalCheckEnergyReconstruction : public framework::Analyzer {
 public:
  EcalCheckEnergyReconstruction(const std::string &name, framework::Process &p)
      : framework::Analyzer(name, p) {}
  ~EcalCheckEnergyReconstruction() {}

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
    const auto simHits =
        event.getCollection<ldmx::SimCalorimeterHit>("EcalSimHits");

    REQUIRE(simHits.size() == 1);

    float truth_energy = simHits.at(0).getEdep();
    ntuple_.setVar<float>("SimEnergy", truth_energy);

    const auto daqDigis{
        event.getObject<ldmx::HgcrocDigiCollection>("EcalDigis")};

    CHECK(daqDigis.getNumDigis() == 1);
    auto daqDigi = daqDigis.getDigi(0);
    ntuple_.setVar<int>("DaqDigi", daqDigi.soi().raw());
    bool is_in_adc_mode = daqDigi.isADC();
    ntuple_.setVar<int>("DaqDigiIsADC", is_in_adc_mode);
    ntuple_.setVar<int>("DaqDigiADC", daqDigi.soi().adc_t());
    ntuple_.setVar<int>("DaqDigiTOT", daqDigi.tot());

    const auto recHits = event.getCollection<ldmx::EcalHit>("EcalRecHits");
    CHECK(recHits.size() == 1);

    auto hit = recHits.at(0);
    ldmx::EcalID id(hit.getID());
    CHECK_FALSE(hit.isNoise());
    CHECK(id.raw() == simHits.at(0).getID());

    double daq_energy{hit.getAmplitude()};
    CHECK_THAT(daq_energy,
        isCloseEnough(truth_energy,MAX_ENERGY_ERROR_DAQ,MAX_ENERGY_PERCENT_ERROR_DAQ));
    ntuple_.setVar<float>("RecEnergy", hit.getAmplitude());

    const auto trigDigis{
        event.getObject<ldmx::HgcrocTrigDigiCollection>("ecalTrigDigis")};
    CHECK(trigDigis.size() == 1);

    auto trigDigi = trigDigis.at(0);
    float tp_energy = 8 * trigDigi.linearPrimitive() * 320. / 1024 * MeV_per_fC;

    CHECK_THAT(tp_energy ,
        isCloseEnough(truth_energy,MAX_ENERGY_ERROR_TP,MAX_ENERGY_PERCENT_ERROR_TP));
    ntuple_.setVar<float>("TrigPrimEnergy", tp_energy);
    ntuple_.setVar<int>("TrigPrimDigiEncoded", trigDigi.getPrimitive());
    ntuple_.setVar<int>("TrigPrimDigiLinear", trigDigi.linearPrimitive());

    return;
  }
};  // EcalCheckEnergyReconstruction

}  // namespace test
}  // namespace ecal

DECLARE_PRODUCER_NS(ecal::test, EcalFakeSimHits)
DECLARE_ANALYZER_NS(ecal::test, EcalCheckEnergyReconstruction)

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
 * @TODO check layer weights are being calculated correctly somehow
 */
TEST_CASE("Ecal Digi Pipeline test", "[Ecal][functionality]") {
  const std::string config_file{"ecal_digi_pipeline_test_config.py"};

  char **args;
  framework::ProcessHandle p;

  framework::ConfigurePython cfg(config_file, args, 0);
  REQUIRE_NOTHROW(p = cfg.makeProcess());
  p->run();
}
