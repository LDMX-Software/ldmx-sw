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
 * @class FakeSimHits
 *
 * Fills the event bus with an EcalSimHits collection with
 * a range of energy hits. These hits are put into unique
 * cells so that we can compare them to the correct energy
 * in one event.
 */
class EcalFakeSimHits : public Producer {

        /// Maximum energy to make a simulated hit for [MeV]
        double maxEnergy_  = 80. ;

        /// Difference between energies to make a sim hit for [MeV]
        double energyStep_ = 0.1 ; //MeV

        /**
         * Minimum energy to make a sim hit for [MeV]
         * Needs to be above readout threshold (after internal EcalDigi's calculation)
         */
        double minEnergy_ = 0.5 ; 

    public:

        EcalFakeSimHits(const std::string &name,Process& p) : Producer( name , p ) { }
        ~EcalFakeSimHits() { }

        void beforeNewRun(RunHeader& header) {
            header.setDetectorName("ldmx-det-v12");
        }

        void produce(Event& event) final override {

            std::vector<SimCalorimeterHit> pretendSimHits;
        
            //fill pretend sim hits with a range of simulated energies
            int cell(0), module(0), layer(0);
            double currEnergy{minEnergy_};
            while ( currEnergy < maxEnergy_ ) {
                //TODO test several contribs
                //  right now, not a big deal because we just do an energy weight average anyways
                SimCalorimeterHit currHit;
                //distribute indices across distince IDs
                //  need to be careful to not exceed max count of subparts
                EcalID id(layer,module,cell);
                currHit.setID( id.raw() );
                currHit.addContrib(
                        -1 //incidentID
                        , -1 // trackID
                        , 0 // pdg ID
                        , currEnergy // edep
                        , 1. //time
                        );
                pretendSimHits.push_back( currHit );
        
                currEnergy += energyStep_;
                cell++;
                if ( cell > 300 ) {
                    cell = 0;
                    module++;
                }
                if ( module > 7 ) {
                    module = 0;
                    layer++;
                }
            }
            
            //needs to be correct collection name
            REQUIRE_NOTHROW(event.add( "EcalSimHits" , pretendSimHits ));

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

            std::map<EcalID,double> correct_energies;
            for ( const auto& hit : simHits ) correct_energies[EcalID(hit.getID())] = hit.getEdep();

            std::vector<EcalHit> recHits;
            REQUIRE_NOTHROW(recHits = event.getCollection<EcalHit>( "EcalRecHits" ));

            for ( const auto& hit : recHits ) {
                EcalID id(hit.getID());
                if ( correct_energies.count(id) > 0 ) {
                    //not a noise hit
                    //argument to epsilon is maximum relative difference allowed
                    //  right now, I set it to 0.05 (5% allowed difference)
                    CHECK_FALSE( hit.isNoise() );
                    CHECK( hit.getAmplitude() == Approx( correct_energies.at(id) ).epsilon(0.05) );
                    correct_energies.erase(id);
                } else {
                    //should be flagged as noise
                    CHECK( hit.isNoise() );
                }
            }

            // did we lose any energies during the processing?
            //  all of the correct eneriges should have been visited and erased in the above loop
            CHECK( correct_energies.size() == 0 );

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
    cf << "p.maxEvents = 1" << std::endl;
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
