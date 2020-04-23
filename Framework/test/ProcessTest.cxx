/**
 * @file ProcessTest.cxx
 * @brief Test the operation of the Process class
 *
 * @author Tom Eichlersmith, University of Minnesota
 */
#include "Exception/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/Process.h"
#include "Framework/EventProcessor.h"
#include "Framework/EventProcessorFactory.h"

using namespace ldmx;

/**
 * Test for ldmx-app function
 *
 * First argument is name of this test.
 * Second argument is tags to group tests together.
 */
TEST_CASE( "Process Functionality Test" , "[Framework][functionality]" ) {

    Process p("test");

    REQUIRE( p.getPassName() == "test" );

    EventProcessor *dP = EventProcessorFactory::getInstance().createEventProcessor( "ldmx::DummyProducer" , "DummyProducer_TEST" , p );
    REQUIRE_FALSE( dP == nullptr );
    std::map< std::string , std::any > params;
    params[ "nParticles" ] = 1;
    params[ "aveEnergy" ] = 10.;
    std::vector<double> dir;
    params[ "direction" ] = dir;
    Parameters parameters;
    parameters.setParameters( params );
    dP->configure( parameters );

    EventProcessor *dA = EventProcessorFactory::getInstance().createEventProcessor( "ldmx::DummyAnalyzer" , "DummyAnalyzer_TEST" , p );
    REQUIRE_FALSE( dA == nullptr );
    params.clear();
    params[ "caloHitCollection" ] = std::string("caloHits");
    parameters.setParameters( params );
    dA->configure( parameters );

    SECTION( "Production Mode" ) {
        //no input files, only output files

        p.setOutputFileName( "test_out.root" );
        p.setEventLimit( 5 );
        p.addToSequence( dP );

        CHECK_NOTHROW( p.run() );
        
        SECTION( "with Analyses" ) {

            p.addToSequence( dA );

            CHECK_NOTHROW( p.run() );
        }

        SECTION( "with drop/keep rules" ) {

            p.addDropKeepRule( "drop .*Hits.*" );

            CHECK_NOTHROW( p.run() );

        }

    }

    SECTION( "Need Input Files" ) {

        p.addToSequence( dP );

        p.setOutputFileName( "test_input0.root" );
        p.setEventLimit( 5 );
        
        CHECK_NOTHROW( p.run() );

        p.setOutputFileName( "test_input1.root" );
        p.setEventLimit( 10 );
        
        CHECK_NOTHROW( p.run() );
    
        p.setOutputFileName( "test_input2.root" );
        p.setEventLimit( 15 );
        
        CHECK_NOTHROW( p.run() );

        SECTION( "Analysis Mode" ) {
            //no output files, only histogram output

            Process anap("TESTana");
            anap.addToSequence( dA );
            anap.setHistogramFileName( "test_histo.root" );
            anap.addFileToProcess( "test_input0.root" );

            anap.run();
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

}
