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
static const int NUM_TEST_SIM_HITS=61;

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
        const double maxEnergy_  = 12.6; //80.;

        /**
         * Minimum energy to make a sim hit for [MeV]
         * Needs to be above readout threshold (after internal EcalDigi's calculation)
         */
        const double minEnergy_ = 6.5; //0.5; 

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

        void analyze(const Event& event) final override {

            std::vector<SimCalorimeterHit> simHits;
            REQUIRE_NOTHROW(simHits = event.getCollection<SimCalorimeterHit>( "EcalSimHits" ));

            std::vector<EcalHit> recHits;
            REQUIRE_NOTHROW(recHits = event.getCollection<EcalHit>( "EcalRecHits" ));

            bool found_real_hit = false;
            for ( const auto& hit : recHits ) {
                EcalID id(hit.getID());
                if ( id.raw() == simHits.at(0).getID() ) {
                    //not a noise hit
                    //argument to epsilon is maximum relative difference allowed
                    CHECK_FALSE( hit.isNoise() );
                    CHECK( hit.getAmplitude() == Approx( simHits.at(0).getEdep() ).epsilon(MAX_ENERGY_PERCENT_ERROR/100) );
                    found_real_hit = true;
                } else {
                    //should be flagged as noise
                    CHECK( hit.isNoise() );
                }
            }

            // did we lose any energies during the processing?
            CHECK( found_real_hit );

            return;
        }
};//EcalCheckEnergyReconstruction

} //test
} //ldmx

DECLARE_PRODUCER_NS(ldmx::test,EcalFakeSimHits)
DECLARE_ANALYZER_NS(ldmx::test,EcalCheckEnergyReconstruction)

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
    cf << "p.maxEvents = " << ldmx::test::NUM_TEST_SIM_HITS << std::endl;
    cf << "from LDMX.Ecal import ecal_hardcoded_conditions" << std::endl;
    cf << "from LDMX.Ecal import digi" << std::endl;
    cf << "p.outputFiles = [ '/tmp/ecal_digi_pipeline_test.root' ]" << std::endl;
    cf << "from LDMX.Ecal import EcalGeometry" << std::endl;
    cf << "geom = EcalGeometry.EcalGeometryProvider.getInstance()" << std::endl;
    cf << "p.sequence = [" << std::endl;
    cf << "    ldmxcfg.Producer('fakeSimHits','ldmx::test::EcalFakeSimHits','Ecal')," << std::endl;
    cf << "    digi.EcalDigiProducer()," << std::endl;
    cf << "    digi.EcalRecProducer()," << std::endl;
    cf << "    ldmxcfg.Analyzer('checkEcalHits','ldmx::test::EcalCheckEnergyReconstruction','Ecal')," << std::endl;
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
