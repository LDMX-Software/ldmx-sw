/**
 * @file FunctionalCoreTest.cxx
 * @brief Test the operation of Framework processing
 *
 * @author Tom Eichlersmith, University of Minnesota
 *  
 */
#include "Exception/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include <cstdio> //for remove
#include "TH1F.h" //for test histogram
#include "TFile.h" //to open and check root files
#include "TTreeReader.h" //to check output event files

#include "Event/EventDef.h"
#include "Framework/Process.h"
#include "Framework/EventProcessor.h"

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
                caloHits.back().setID( i_event*10 + i );
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

        void onProcessStart() final override {
            REQUIRE_NOTHROW(getHistoDirectory());
            test_hist_ = new TH1F( 
                    "test_hist_" , "Test Histogram",
                    101, -50, 50 );
            test_hist_->SetCanExtend(TH1::kAllAxes);
        }

        void analyze(const Event& event) final override {

            int i_event = event.getEventNumber();

            REQUIRE( i_event > 0 );

            std::vector<CalorimeterHit> caloHits;
            REQUIRE_NOTHROW(caloHits = event.getCollection<CalorimeterHit>( "TestCollection" ));

            CHECK( caloHits.size() == i_event );
            for ( unsigned int i = 0; i < caloHits.size(); i++ ) {
                CHECK( caloHits.at(i).getID() == i_event*10 + i );
                test_hist_->Fill( caloHits.at(i).getID() );
            }

            HcalVetoResult vetoRes;
            REQUIRE_NOTHROW(vetoRes = event.getObject<HcalVetoResult>( "TestObject" ));

            auto maxPEHit{vetoRes.getMaxPEHit()};

            CHECK( maxPEHit.getID()     == i_event );
            CHECK( vetoRes.passesVeto() == (i_event%2==0) );

            return;
        }

    private:

        /// test histogram filled with event indices
        TH1F *test_hist_;
};//TestAnalyzer

/**
 * Name for the test histogram file to be shared everywhere in
 * the ldmx-test executable
 */
const char * TEST_HISTOGRAM_FILENAME = "test_histo.root";

/**
 * @func closeHistogramFile
 *
 * @param[in] correct_num_entries in the histogram
 *
 * Runs a variety of CHECKs to make sure the
 * histogram is what we expect it to be.
 */
void closeHistogramFile( const int &correct_num_entries ) {
    
    TFile file( TEST_HISTOGRAM_FILENAME );
    TDirectory *d = (TDirectory*)file.Get( "TestAnalyzer" );
    REQUIRE( d != nullptr ); //unable to get directory
    TH1F* h = (TH1F*)d->Get("test_hist_");
    REQUIRE( h != nullptr ); //unable to get the histogram
    
    CHECK( h->GetEntries() == correct_num_entries );

    file.Close();

    CHECK( remove( TEST_HISTOGRAM_FILENAME ) == 0 ); //delete file

    return;
}

/**
 * @func checkEventFile
 *
 * Looks through output Events ttree and checks that the TestCollection,
 * TestObject, and EventHeader follow the structure that the producer
 * made.
 *
 * @param[in] filename name of event file to check
 * @param[in] passname name of pass to check
 * @param[in] correct_num_entries that the event file should have
 */
void checkEventFile( const std::string &filename , const std::string &passname, const int &correct_num_entries ) {

    TFile f( filename.c_str() );

    TTreeReader r( "LDMX_Events" , &f );

    //TODO check if tree loaded
    //CHECK( r.GetEntryStatus() != TTreeReader::EEntryStatus::kEntryNoTree );

    TTreeReaderValue<std::vector<CalorimeterHit>> collection( r , ("TestCollection_"+passname).c_str() );
    TTreeReaderValue<HcalVetoResult> object( r , ("TestObject_"+passname).c_str() );
    TTreeReaderValue<EventHeader> header( r , "EventHeader" );

    //TODO check if tree actually has these branches

    CHECK( r.GetEntries(true) == correct_num_entries );

    while( r.Next() ) {
        CHECK( collection->size() == header->getEventNumber() );
        CHECK( object->getMaxPEHit().getID() == header->getEventNumber() );
        for ( unsigned int i = 0; i < collection->size(); i++ ) {
            CHECK( collection->at(i).getID() == header->getEventNumber()*10+i );
        }
    }
     
    f.Close();

    return;
}

/**
 * Test for C++ Framework processing.
 *
 * This test is aimed at checking that core functionalities are operational.
 * Python configuration, Simulation, and other add-on functionalities
 * are tested separately.
 *
 * What does this even test?
 *  - Event::add an object and a vector of objects (changing size and content)
 *  - Event::get an object and a vector of objects (changing size and content)
 *  - Event can switch to different input tree (Multiple Input Files)
 *  - Creating and filling a histogram
 *  - Reading from input file(s)
 *  - Production Mode (no input files)
 *  - Analysis Mode (no output event files, only output histogram file)
 *  - Merge Mode (several input files to one output file)
 *  - N-to-N Mode (several input files to several output files)
 *  - writing histogram to a file
 *  - Writing to an output file
 *  - TODO drop/keep rules for event bus passengers
 *  - TODO writing and reading run headers
 *  - TODO skimming events (only keeping events meeting a certain criteria)
 */
TEST_CASE( "Core Framework Functionality" , "[Framework][functionality]" ) {

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
            p.setHistogramFileName( TEST_HISTOGRAM_FILENAME );
            p.addToSequence( anaHdl );
            CHECK_NOTHROW( p.run() );
            closeHistogramFile( 1+2+3 );
        }

        checkEventFile( "test_out.root" , "test" , 3 );
        CHECK( remove( "test_out.root" ) == 0 );
    }//Production Mode

    SECTION( "Need Input Files" ) {

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
    
        //check that input files were created fine
        checkEventFile( "test_input0.root" , "makeInputs" , 2 );
        checkEventFile( "test_input1.root" , "makeInputs" , 3 );
        checkEventFile( "test_input2.root" , "makeInputs" , 4 );

        SECTION( "Analysis Mode" ) {
            //no output files, only histogram output

            p.addToSequence( anaHdl );
            p.setHistogramFileName( TEST_HISTOGRAM_FILENAME );

            SECTION( "one input file" ) {
                p.addFileToProcess( "test_input0.root" );
                p.run();

                closeHistogramFile( 1+2 );
            }

            
            SECTION( "multiple input files" ) {
                p.addFileToProcess( "test_input0.root" );
                p.addFileToProcess( "test_input1.root" );
                p.addFileToProcess( "test_input2.root" );
                p.run();

                closeHistogramFile( 1+2+1+2+3+1+2+3+4 );
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
                p.setHistogramFileName( TEST_HISTOGRAM_FILENAME );
    
                p.run();

                closeHistogramFile( 1+2 + 1+2+3 + 1+2+3+4 );
            }
    
            SECTION( "with producers" ) {
                p.addToSequence( proHdl );
                p.run();

                checkEventFile( "test_merge.root" , "test" , 2+3+4 );
            }

            //checks that input collections were copied over correctly
            checkEventFile( "test_merge.root" , "makeInputs" , 2+3+4 );
            //CHECK( remove( "test_merge.root" ) == 0 );
        } //Merge Mode

        SECTION( "N-to-N Mode" ) {
            //many input files to many output files
            p.addFileToProcess( "test_input0.root" );
            p.addFileToProcess( "test_input1.root" );
            p.addFileToProcess( "test_input2.root" );

            p.addOutputFileName( "test_output0.root" );
            p.addOutputFileName( "test_output1.root" );
            p.addOutputFileName( "test_output2.root" );

            SECTION( "no processors" ) {
                p.run();
            }

            SECTION( "with analyzer" ) {
                p.addToSequence( anaHdl );
                p.setHistogramFileName( TEST_HISTOGRAM_FILENAME );
    
                p.run();

                closeHistogramFile( 1+2+1+2+3+1+2+3+4 );
            }
    
            SECTION( "with producers" ) {
                p.addToSequence( proHdl );
                p.run();

                //checks that produced objects were written correctly
                checkEventFile( "test_output0.root" , "test" , 2 );
                checkEventFile( "test_output1.root" , "test" , 3 );
                checkEventFile( "test_output2.root" , "test" , 4 );
            }

            //checks that input pass was copied over correctly
            checkEventFile( "test_output0.root" , "makeInputs" , 2 );
            //CHECK( remove( "test_output0.root" ) == 0 );

            checkEventFile( "test_output1.root" , "makeInputs" , 3 );
            //CHECK( remove( "test_output1.root" ) == 0 );

            checkEventFile( "test_output2.root" , "makeInputs" , 4 );
            //CHECK( remove( "test_output2.root" ) == 0 );

        } // N-to-N Mode

        //cleanup
        //  delete any files that this test produced
        //  test_out.root, test_input{0,1,2}.root, test_merge.root, test_histo.root
        CHECK( remove( "test_input0.root" ) == 0 );
        CHECK( remove( "test_input1.root" ) == 0 );
        CHECK( remove( "test_input2.root" ) == 0 );

    } //need input files


}//process test
