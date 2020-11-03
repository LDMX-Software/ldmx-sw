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
 * Maximum percent error that a single hit
 * can be reconstructed with before failing the test.
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_PERCENT_ERROR=2.5;

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
static const int NUM_TEST_SIM_HITS=795;
//static const int NUM_TEST_SIM_HITS=61;

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

        /// Maximum energy to make a simulated hit for [MeV]
        const double maxEnergy_  = 80.;
        //const double maxEnergy_  = 12.6; 

        /**
         * Minimum energy to make a sim hit for [MeV]
         * Needs to be above readout threshold (after internal EcalDigi's calculation)
         */
        const double minEnergy_ = 0.5;
        //const double minEnergy_ = 6.5;

        /// Difference between energies to make a sim hit for [MeV]
        const double energyStep_ = (maxEnergy_-minEnergy_)/(NUM_TEST_SIM_HITS); //MeV

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

            currEnergy_ += energyStep_;
            
            return;
        }
}; //EcalFakeSimHits

/**
 * @class EcalCheckEnergyReconstruction
 *
 * Checks
 * - Amplitude of EcalRecHit matches SimCalorimeterHit EDep with the same ID
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

            float truth_energy = simHits.at(0).getEdep();
            ntuple_.setVar<float>("SimEnergy",truth_energy);

            const auto recHits = event.getCollection<EcalHit>( "EcalRecHits" );

            bool found_real_hit = false;
            for ( const auto& hit : recHits ) {
                EcalID id(hit.getID());
                if ( id.raw() == simHits.at(0).getID() ) {
                    //not a noise hit
                    //argument to epsilon is maximum relative difference allowed
                    CHECK_FALSE( hit.isNoise() );
                    CHECK( hit.getAmplitude() == Approx(truth_energy).epsilon(MAX_ENERGY_PERCENT_ERROR/100) );
                    ntuple_.setVar<float>("RecEnergy",hit.getAmplitude());
                    found_real_hit = true;
                } else {
                    //should be flagged as noise
                    CHECK( hit.isNoise() );
                }
            }

            // did we lose any energies during the processing?
            CHECK( found_real_hit );

            const auto trigDigis{event.getObject<HgcrocTrigDigiCollection>("ecalTrigDigis")};
            for (const auto& digi : trigDigis ) {
                //TODO make this work when we include noise in testing
                
                float tp_energy = 8 * digi.linearPrimitive() * 320./1024 * 0.130 / (37000.*(0.162/1000));
                //CHECK( tp_energy == Approx(truth_energy).epsilon(MAX_ENERGY_PERCENT_ERROR/100) );
                ntuple_.setVar<float>("TrigPrimEnergy",tp_energy);
                ntuple_.setVar<int>("TrigPrimDigiEncoded",digi.getPrimitive());
                ntuple_.setVar<int>("TrigPrimDigiLinear",digi.linearPrimitive());
            }

            const auto digis{event.getObject<HgcrocDigiCollection>("EcalDigis")};
            for ( unsigned int iDigi = 0; iDigi < digis.getNumDigis(); iDigi++ ) {
                //TODO make this work when we include noise in testing
                auto digi = digis.getDigi( iDigi );
                ntuple_.setVar<int>("DaqDigi",digi.soi().raw());
            }

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
 *  Close is defined right now to be a maximum of 5% relative difference.
 *
 * Checks
 *  - Keep reconstructed energy depositied within 5% of simulated value
 *  - Mark any pure noise hits as noise
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
    cf << "p.sequence = [" << std::endl;
    cf << "    ldmxcfg.Producer('fakeSimHits','ldmx::test::ecal::EcalFakeSimHits','Ecal')," << std::endl;
    cf << "    digi.EcalDigiProducer()," << std::endl;
    cf << "    ecal_trig_digi.EcalTrigPrimDigiProducer()," << std::endl;
    cf << "    digi.EcalRecProducer()," << std::endl;
    cf << "    ldmxcfg.Analyzer('checkEcalHits','ldmx::test::ecal::EcalCheckEnergyReconstruction','Ecal')," << std::endl;
    cf << "    ]" << std::endl;

    /* debug printing during run
     */
    cf << "p.termLogLevel = 1" << std::endl;
    cf << "p.logFrequency = 1" << std::endl;

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
