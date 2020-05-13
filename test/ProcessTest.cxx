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
 * @class TestProducer
 * Bare producer that creates a collection and an object and puts them 
 * on the event bus.
 *
 * Checks that the Event::add function does not throw any errors.
 */
class TestProducer : public Producer {

    public:

        TestProducer(Process& p) : Producer( "TestProducer" , p ) { }
        ~TestProducer() { }

        void produce(Event& event) final override {

            int i_event = event.getEventNumber();

            REQUIRE( i_event > 0 );

            std::vector<CalorimeterHit> caloHits;
            for ( int i = 0; i < i_event; i++ ) {
                caloHits.emplace_back();
                caloHits.back().setID( i_event*1000 + i );
            }

            REQUIRE_NOTHROW( event.add( "TestCollection" , caloHits ) );

            HcalHit maxPEHit;
            maxPEHit.setID( i_event );

            HcalVetoResult res;
            res.setMaxPEHit( maxPEHit );
            res.setVetoResult( i_event % 2 == 0 );

            REQUIRE_NOTHROW( event.add( "TestObject" , res ) );

            return;
        }
};//TestProducer

/**
 * @class TestAnalyzer
 * Bare analyzer that looks for objects matching what the TestProducer put in.
 *
 * Checks for the correct number and contents.
 * Checks that Event::getCollection and Event::getObject don't throw errors.
 */
class TestAnalyzer : public Analyzer {

    public:

        TestAnalyzer(Process& p) : Analyzer( "TestAnalyzer" , p ) { }
        ~TestAnalyzer() { }

        void analyze(const Event& event) final override {

            int i_event = event.getEventNumber();

            REQUIRE( i_event > 0 );

            std::vector<CalorimeterHit> caloHits;
            REQUIRE_NOTHROW(caloHits = event.getCollection<CalorimeterHit>( "TestCollection" ));

            CHECK( caloHits.size() == i_event );
            for ( int i = 0; i < caloHits.size(); i++ ) {
                CHECK( caloHits.at(i).getID() == i_event*1000 + i );
            }

            HcalVetoResult vetoRes;
            REQUIRE_NOTHROW(vetoRes = event.getObject<HcalVetoResult>( "TestObject" ));

            auto maxPEHit{vetoRes.getMaxPEHit()};

            CHECK( maxPEHit.getID()     == i_event );
            CHECK( vetoRes.passesVeto() == (i_event%2==0) );

            return;
        }
};//TestAnalyzer

/**
 * Test for ldmx-app function
 *
 * First argument is name of this test.
 * Second argument is tags to group tests together.
 */
TEST_CASE( "Process Functionality Test" , "[Framework][functionality]" ) {

    Process p("test");

    REQUIRE( p.getPassName() == "test" );

    TestProducer pro( p );
    TestAnalyzer ana( p );

    auto proHdl = &pro;
    auto anaHdl = &ana;

    SECTION( "Production Mode" ) {
        //no input files, only output files

        p.setOutputFileName( "test_out.root" );
        p.setEventLimit( 3 );
        p.addToSequence( proHdl );

        SECTION( "only producers" ) {
            CHECK_NOTHROW( p.run() );
        }
        
        SECTION( "with Analyses" ) {
            p.addToSequence( anaHdl );
            CHECK_NOTHROW( p.run() );
        }
    }//Production Mode

    /**************************************************************************
     * Make input files using a different process (and different pass name)
     *************************************************************************/
    Process makeInputs( "makeInputs" );
    TestProducer inputPro( makeInputs );
    auto inputProHdl = &inputPro;
    makeInputs.addToSequence( inputProHdl );
    makeInputs.setOutputFileName( "test_input0.root" );
    makeInputs.setEventLimit( 2 );
        
    CHECK_NOTHROW( makeInputs.run() );
    makeInputs.setOutputFileName( "test_input1.root" );
    makeInputs.setEventLimit( 3 );
        
    CHECK_NOTHROW( makeInputs.run() );
   
    makeInputs.setOutputFileName( "test_input2.root" );
    makeInputs.setEventLimit( 4 );
        
    CHECK_NOTHROW( makeInputs.run() );

    SECTION( "Need Input Files" ) {

        SECTION( "Analysis Mode" ) {
            //no output files, only histogram output

            p.addToSequence( anaHdl );
            p.setHistogramFileName( "test_histo.root" );

            SECTION( "one input file" ) {
                p.addFileToProcess( "test_input0.root" );
                p.run();
            }

            SECTION( "multiple input files" ) {
                p.addFileToProcess( "test_input0.root" );
                p.addFileToProcess( "test_input1.root" );
                p.addFileToProcess( "test_input2.root" );
                p.run();
            }
        }//Analysis Mode

        SECTION( "Merge Mode" ) {
            //many input files to one output file
            
            p.addFileToProcess( "test_input0.root" );
            p.addFileToProcess( "test_input1.root" );
            p.addFileToProcess( "test_input2.root" );

            p.setOutputFileName( "test_merge.root" );

            SECTION( "no processors" ) {
                p.run();
            }
    
            SECTION( "with analyzers" ) {
                p.addToSequence( anaHdl );
                p.setHistogramFileName( "test_histo.root" );
    
                p.run();
            }
    
            SECTION( "with producers" ) {
                p.addToSequence( proHdl );
                p.run();
            }

        } //Merge Mode
    } //need input files
}//process test
