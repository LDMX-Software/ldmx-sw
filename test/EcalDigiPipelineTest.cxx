/**
 * @file EcalDigiPipelineTest.cxx
 * @brief Test to make sure input sim energies are close to output rec energies
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/EventProcessor.h"
#include "Framework/Process.h" 
#include "Framework/ConfigurePython.h"
#include "Event/EventDef.h" //Need event bus passengers
#include "DetDescr/EcalID.h" //creating unique cell IDs

#include <fstream> //to write config file

namespace ldmx {
namespace test {
namespace ecal {

/**
 * Energy deposited in our silicon sensors by one MIP on average.
 * [MeV]
 */
static const double MIP_SI_ENERGY=0.130;

/**
 * Conversion between deposited charge and deposited energy
 * [MeV/fC]
 *
 * charge [fC] * (1000 electrons / 0.162 fC) * (1 MIP / 37 000 electrons) * (0.130 MeV / 1 MIP)
 */
static const double MeV_per_fC=MIP_SI_ENERGY/(37*0.162);

/**
 * Maximum percent error that a single hit
 * can be reconstructed with before failing the test
 * if above the tot threshold.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_PERCENT_ERROR_DAQ_TOT_MODE=5.;

/**
 * Maximum percent error that a single hit can be
 * estimated at the trigger primitive level when
 * reading out in TOT mode.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the
 * energy estimated from trigger primitives.
 */
static const double MAX_ENERGY_PERCENT_ERROR_TP_TOT_MODE=20.;

/**
 * Maximum absolute error that a single hit
 * can be reconstructed with before failing the test
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_ERROR_DAQ_ADC_MODE=MIP_SI_ENERGY/2;

/**
 * Maximum absolute error that a single hit
 * can be estimated at the trigger primitive level
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the
 * energy estimated from trigger primitives.
 */
static const double MAX_ENERGY_ERROR_TP_ADC_MODE=2*MIP_SI_ENERGY; 

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
static const int NUM_TEST_SIM_HITS=1600;

/**
 * Should the sim/rec/tp energies be ntuplized
 * for your viewing?
 */
static const bool NTUPLIZE_ENERGIES=true;

/**
 * @class FakeSimHits
 *
 * Fills the event bus with an EcalSimHits collection with
 * a range of energy hits. These hits are put into unique
 * cells so that we can compare them to the correct energy
 * in one event.
 */
class EcalFakeSimHits : public Producer {

        /**
         * Maximum energy to make a simulated hit for [MeV]
         *
         * The maximum value to be readout is 4096 TDC which
         * is equivalent to 4000fC deposited charge which
         * is equivalent to ~86MeV.
         */
        const double maxEnergy_  = 4000.*MeV_per_fC;

        /**
         * Minimum energy to make a sim hit for [MeV]
         * Needs to be above readout threshold (after internal EcalDigi's calculation)
         *
         * One MIP is ~0.13 MeV, so we choose that.
         */
        const double minEnergy_ = MIP_SI_ENERGY;

        /**
         * Best possible resolution of our detector when reading out in ADC mode is
         *
         * (320 fC)/(1024 counts)(0.130 MeV/MIP)/(37*0.162 fC/MIP) = 6.7e-3 MeV
         */
        const double adcEnergyStep_ = (320./1024)*MeV_per_fC;

        /**
         * TOT threshold in MeV
         *
         * About 6.5 MeV
         */
        const double totThreshold_ = 50*MIP_SI_ENERGY;

        /**
         * Best possible resolution for our detector when reading out in
         * the lower TOT mode (TDC < 512) is
         *
         * (4000 fC / 4096 counts)*(0.130 MeV per MIP / (37*0.162) fC per MIP) = 2.1e-2 MeV
         */
        const double totLowEnergyStep_ = (4000./4096)*MeV_per_fC;

        /**
         * Threshold between low energy TOT and high energy TOT in MeV
         *
         * About 10.7 MeV
         */
        const double totHighThreshold_ = 512*totLowEnergyStep_;

        /**
         * Best possible resolution for our detector when reading out in
         * the higher TOT mode (TDC > 512) is eight times higher than
         * the lower TDC mode because we throw away the lowest 3 bits.
         */
        const double totHighEnergyStep_ = 8*totLowEnergyStep_;

        /// current energy of the sim hit we are on
        double currEnergy_ = minEnergy_;

    public:

        EcalFakeSimHits(const std::string &name,Process& p) : Producer( name , p ) { }
        ~EcalFakeSimHits() { }

        void beforeNewRun(RunHeader& header) {
            header.setDetectorName("ldmx-det-v12");
        }

        void produce(Event& event) final override {

            //put in a single sim hit
            std::vector<SimCalorimeterHit> pretendSimHits(1);

            EcalID id(0,0,0);
            pretendSimHits[0].setID( id.raw() );
            pretendSimHits[0].addContrib(
                        -1 //incidentID
                        , -1 // trackID
                        , 0 // pdg ID
                        , currEnergy_ // edep
                        , 1. //time
                        );

            //needs to be correct collection name
            REQUIRE_NOTHROW(event.add( "EcalSimHits" , pretendSimHits ));

            if (currEnergy_ < totThreshold_ ) currEnergy_ += adcEnergyStep_;
            else if (currEnergy_ < totHighThreshold_ ) currEnergy_ += totLowEnergyStep_;
            else currEnergy_ += totHighEnergyStep_;
            
            return;
        }
}; //EcalFakeSimHits

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
class EcalCheckEnergyReconstruction : public Analyzer {

    public:

        EcalCheckEnergyReconstruction(const std::string& name,Process& p) : Analyzer( name , p ) { }
        ~EcalCheckEnergyReconstruction() { }

        void onProcessStart() final override {

            getHistoDirectory();
            ntuple_.create("EcalDigiTest");
            ntuple_.addVar<float>("EcalDigiTest","SimEnergy");
            ntuple_.addVar<float>("EcalDigiTest","RecEnergy");
            ntuple_.addVar<float>("EcalDigiTest","TrigPrimEnergy");

            ntuple_.addVar<int>("EcalDigiTest","DaqDigi");
            ntuple_.addVar<int>("EcalDigiTest","TrigPrimDigiEncoded");
            ntuple_.addVar<int>("EcalDigiTest","TrigPrimDigiLinear" );

        }

        void analyze(const Event& event) final override {

            const auto simHits = event.getCollection<SimCalorimeterHit>( "EcalSimHits" );

            REQUIRE( simHits.size() == 1 );

            float truth_energy = simHits.at(0).getEdep();
            ntuple_.setVar<float>("SimEnergy",truth_energy);

            const auto daqDigis{event.getObject<HgcrocDigiCollection>("EcalDigis")};

            CHECK( daqDigis.getNumDigis() == 1 );
            auto daqDigi = daqDigis.getDigi(0);
            ntuple_.setVar<int>("DaqDigi",daqDigi.soi().raw());
            bool is_in_adc_mode = daqDigi.isADC();

            const auto recHits = event.getCollection<EcalHit>( "EcalRecHits" );
            CHECK( recHits.size() == 1 );

            auto hit = recHits.at(0);
            EcalID id(hit.getID());
            CHECK_FALSE( hit.isNoise() );
            CHECK( id.raw() == simHits.at(0).getID() );

            //define target energy by using the settings at the top
            auto target_daq_energy = Approx(truth_energy).epsilon(MAX_ENERGY_PERCENT_ERROR_DAQ_TOT_MODE/100);
            if (is_in_adc_mode) target_daq_energy = Approx(truth_energy).margin(MAX_ENERGY_ERROR_DAQ_ADC_MODE);

            CHECK( hit.getAmplitude() == target_daq_energy );
            ntuple_.setVar<float>("RecEnergy",hit.getAmplitude());

            const auto trigDigis{event.getObject<HgcrocTrigDigiCollection>("ecalTrigDigis")};
            CHECK( trigDigis.size() == 1 );
            
            auto trigDigi = trigDigis.at(0);
            float tp_energy = trigDigi.linearPrimitive() * MeV_per_fC; //MeV equivalent to counts in primitive
            if (daqDigi.isADC()) tp_energy *= 8*320./1024; //adc gain
            else                 tp_energy *= 4000./4096;  //tot gain

            auto target_tp_energy = Approx(truth_energy).epsilon(MAX_ENERGY_PERCENT_ERROR_TP_TOT_MODE/100);
            if (is_in_adc_mode) target_tp_energy = Approx(truth_energy).margin(MAX_ENERGY_ERROR_TP_ADC_MODE);

            CHECK( tp_energy == target_tp_energy );
            ntuple_.setVar<float>("TrigPrimEnergy",tp_energy);
            ntuple_.setVar<int>("TrigPrimDigiEncoded",trigDigi.getPrimitive());
            ntuple_.setVar<int>("TrigPrimDigiLinear",trigDigi.linearPrimitive());

            return;
        }
};//EcalCheckEnergyReconstruction

} //ecal
} //test
} //ldmx

DECLARE_PRODUCER_NS(ldmx::test::ecal,EcalFakeSimHits)
DECLARE_ANALYZER_NS(ldmx::test::ecal,EcalCheckEnergyReconstruction)

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
TEST_CASE( "Ecal Digi Pipeline test" , "[Ecal][functionality]" ) {

    const std::string config_file{"/tmp/ecal_digi_pipeline_test.py"};
    std::ofstream cf( config_file );

    cf << "from LDMX.Framework import ldmxcfg" << std::endl;
    cf << "p = ldmxcfg.Process( 'testEcalDigis' )" << std::endl;
    cf << "p.maxEvents = " << ldmx::test::ecal::NUM_TEST_SIM_HITS << std::endl;
    cf << "from LDMX.Ecal import ecal_hardcoded_conditions" << std::endl;
    cf << "from LDMX.Ecal import digi, ecal_trig_digi" << std::endl;
    cf << "p.outputFiles = [ '/tmp/ecal_digi_pipeline_test.root' ]" << std::endl;
    cf << "p.histogramFile = '" << (ldmx::test::ecal::NTUPLIZE_ENERGIES ? "" : "/tmp/") << "ecal_digi_pipeline_test.root'" << std::endl;
    cf << "from LDMX.Ecal import EcalGeometry" << std::endl;
    cf << "geom = EcalGeometry.EcalGeometryProvider.getInstance()" << std::endl;
    cf << "ecalDigis = digi.EcalDigiProducer()" << std::endl;
    cf << "ecalDigis.hgcroc.noise = False" << std::endl; //turn off noise for testing purposes
    cf << "p.sequence = [" << std::endl;
    cf << "    ldmxcfg.Producer('fakeSimHits','ldmx::test::ecal::EcalFakeSimHits','Ecal')," << std::endl;
    cf << "    ecalDigis," << std::endl;
    cf << "    ecal_trig_digi.EcalTrigPrimDigiProducer()," << std::endl;
    cf << "    digi.EcalRecProducer()," << std::endl;
    cf << "    ldmxcfg.Analyzer('checkEcalHits','ldmx::test::ecal::EcalCheckEnergyReconstruction','Ecal')," << std::endl;
    cf << "    ]" << std::endl;

    /* debug printing during run
    cf << "p.termLogLevel = 1" << std::endl;
    cf << "p.logFrequency = 1" << std::endl;
     */

    /* debug pause before running
    cf << "p.pause()" << std::endl;
    */

    cf.close();

    char **args;
    ldmx::ProcessHandle p;

    ldmx::ConfigurePython cfg( config_file , args , 0 );
    REQUIRE_NOTHROW(p = cfg.makeProcess());
    p->run();
}
