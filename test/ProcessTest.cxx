/**
 * @file ProcessTest.cxx
 * @brief Test the operation of the Process class
 *
 * @author Tom Eichlersmith, University of Minnesota
 */
#include "Exception/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/Process.h"

/**
 * Make and configure a dummy producer
 * Owned and deleted by Process
 */
EventProcessor* getDummyProducer() {
    EventProcessor *dP = EventProcessorFactory::getInstance().createEventProcessor( "ldmx::DummyProducer" , "test_DummyProducer" );
    Parameters parameters;
    parameters.setParameters({
            { "nParticles" , 1 },
            { "aveEnergy" , 10. },
            { "direction" , { 0., 0., 0. }}
            });
    dP->configure( parameters );
    return dP;
}

/**
 * Test for ldmx-app function
 *
 * First argument is name of this test.
 * Second argument is tags to group tests together.
 */
TEST_CASE( "Process Functionality Test" , "[Framework][functionality]" ) {

    Process p("test");

    REQUIRE( p.getPassName() == "test" );

    SECTION( "Exceptions" ) {

        SECTION( "Not input/output files" ) {
            //should throw an exception that
            //nothing has been setup
            CHECK_THROWS( p.run() );
        }

        SECTION( "Only output file without number of events" ) {
            p.addOutputFileName( "test_out.root" );
    
            //should throw that no events
            //to run over
            CHECK_THROWS( p.run() );
        }

        SECTION( "Input file doesn't exist." ) {
            p.addInputFileName( "test_in_does_not_exist.root" );
    
            CHECK_THROWS( p.run() );
        }

        SECTION( "Mis-matching Input and Output file numbers." ) {
            p.addOutFileName( "test_out_0.root" );
            p.addOutFileName( "test_out_1.root" );
            p.addOutFileName( "test_out_2.root" );

            p.addInputFileName( "test_in.root" );

            CHECK_THROWS( p.run() );
        }

    }

    SECTION( "Production Mode" ) {
        //no input files, only output files

        p.setOutputFileName( "test_out.root" );
        p.setEventLimit( 5 );
        p.addToSequence( getDummyProducer() );

        CHECK_NOTHROW( p.run() );
        
        SECTION( "with Analyses" ) {

        }

        SECTION( "with drop/keep rules" ) {

        }

    }

    SECTION( "Analysis Mode" ) {
        //no output files, only histogram output

        SECTION( "with drop/keep rules" ) {

        }

    }

    SECTION( "Merge Mode" ) {
        //many input files to one output file

        SECTION( "with Analyses" ) {

        }

        SECTION( "with drop/keep rules" ) {

        }

    }

    SECTION( "N to N Mode" ) {
        //many input files to many output files

        SECTION( "with Analyses" ) {

        }

        SECTION( "with drop/keep rules" ) {

        }

    }

}
