
#include "catch.hpp"  //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "DetDescr/EcalID.h"  //creating unique cell IDs
#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"

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
 * charge [fC] * (1000 electrons / 0.162 fC) * (1 MIP / 37 000 electrons) *
 * (0.130 MeV / 1 MIP)
 */
static const double MeV_per_fC = MIP_SI_ENERGY / (37 * 0.162);

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
 * Maximum percent error that a single hit can be
 * estimated at the trigger primitive level when
 * reading out in TOT mode.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the
 * energy estimated from trigger primitives.
 */
static const double MAX_ENERGY_PERCENT_ERROR_TP_TOT_MODE = 10.;

/**
 * Maximum absolute error that a single hit
 * can be reconstructed with before failing the test
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_ERROR_DAQ_ADC_MODE = MIP_SI_ENERGY / 2;

/**
 * Maximum absolute error that a single hit
 * can be estimated at the trigger primitive level
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the
 * energy estimated from trigger primitives.
 */
static const double MAX_ENERGY_ERROR_TP_ADC_MODE = 2 * MIP_SI_ENERGY;

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
 * @class FakeSimHits
 *
 * Fills the event bus with an EcalSimHits collection with
 * a range of energy hits. These hits are put into unique
 * cells so that we can compare them to the correct energy
 * in one event.
 */
class EcalFakeSimHits : public ldmx::Producer {
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
  EcalFakeSimHits(const std::string &name, ldmx::Process &p)
      : ldmx::Producer(name, p) {}
  ~EcalFakeSimHits() {}

  void beforeNewRun(ldmx::RunHeader &header) {
    header.setDetectorName("ldmx-det-v12");
  }

  void produce(ldmx::Event &event) final override {
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
class EcalCheckEnergyReconstruction : public ldmx::Analyzer {
 public:
  EcalCheckEnergyReconstruction(const std::string &name, ldmx::Process &p)
      : ldmx::Analyzer(name, p) {}
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

  void analyze(const ldmx::Event &event) final override {
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

    // define target energy by using the settings at the top
    auto target_daq_energy =
        Approx(truth_energy)
            .epsilon(MAX_ENERGY_PERCENT_ERROR_DAQ_TOT_MODE / 100);
    if (is_in_adc_mode)
      target_daq_energy =
          Approx(truth_energy).margin(MAX_ENERGY_ERROR_DAQ_ADC_MODE);

    CHECK(hit.getAmplitude() == target_daq_energy);
    ntuple_.setVar<float>("RecEnergy", hit.getAmplitude());

    const auto trigDigis{
        event.getObject<ldmx::HgcrocTrigDigiCollection>("ecalTrigDigis")};
    CHECK(trigDigis.size() == 1);

    auto trigDigi = trigDigis.at(0);
    float tp_energy = 8 * trigDigi.linearPrimitive() * 320. / 1024 * MeV_per_fC;

    auto target_tp_energy =
        Approx(truth_energy)
            .epsilon(MAX_ENERGY_PERCENT_ERROR_TP_TOT_MODE / 100);
    if (is_in_adc_mode)
      target_tp_energy =
          Approx(truth_energy).margin(MAX_ENERGY_ERROR_TP_ADC_MODE);

    CHECK(tp_energy == target_tp_energy);
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
  ldmx::ProcessHandle p;

  ldmx::ConfigurePython cfg(config_file, args, 0);
  REQUIRE_NOTHROW(p = cfg.makeProcess());
  p->run();
}
