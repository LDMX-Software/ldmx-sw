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
 * @class isGoodHistogramFile
 *
 * Runs a variety of checks to make sure the
 * histogram in the input filename is what we expect it to be.
 *
 * Does NOT check that the entries in the histogram are correct,
 * just makes sure that there is the correct number.
 *
 * Does check:
 * - The input file name is readable
 * - The input file has the directory "TestAnalyzer"
 * - The directory has the histogram "test_hist_"
 * - The histogram ahs the correct number of entries
 */
class isGoodHistogramFile : public Catch::MatcherBase<std::string> {
    private:
        /// Correct number of entries
        int correctGetEntries_;

    public:

        /**
         * Constructor
         *
         * Sets the correct event indices
         */
        isGoodHistogramFile( int const& n ) : correctGetEntries_(n) { }

        /**
         * Performs the test for this matcher
         *
         * Opens up the histogram file and makes sure of a few things.
         * - The histogram 'test_hist_' is in the 'TestAnalyzer' directory
         * - The histogram has the correct number of entries
         */
        bool match( const std::string& filename ) const override {
            
            //Open file
            TFile *f = TFile::Open( filename.c_str() );
            if (!f) return false;
            TDirectory *d = (TDirectory*)f->Get( "TestAnalyzer" );
            if (!d) return false;
            TH1F* h = (TH1F*)d->Get("test_hist_");
            if (!h) return false;

            return ( h->GetEntries() == correctGetEntries_ );
        }

        /**
         * Describe this matcher in a helpful, human-readable way.
         *
         * This string is written as if stating a fact about
         * the object it is matching.
         */
        virtual std::string describe() const override {
            std::ostringstream ss;
            ss << "has the histogram 'TestAnalyzer/test_hist_' with the number of entries " << correctGetEntries_;
            return ss.str();
        }
}; //isGoodHistogramFile

/**
 * @class isGoodEventFile
 *
 * Looks through output Events ttree and checks that the TestCollection,
 * TestObject, and EventHeader follow the structure that the producer
 * made.
 *
 */
class isGoodEventFile : public Catch::MatcherBase<std::string> {
    private:
        /// pass name to check the collection and/or object for
        std::string pass_;

        /// correct number of entries in the event ttree
        int entries_;

        /// should we check for the collection?
        bool checkCollection_;

        /// should we check for the object?
        bool checkObject_;

    public:

        /**
         * Constructor
         *
         * Sets the correct number of entries and the other checking parameters
         */
        isGoodEventFile( const std::string& pass , const int& entries , bool checkColl = true , bool checkObj = true )
            : pass_(pass), entries_(entries), checkCollection_(checkColl), checkObject_(checkObj) { }

        /**
         * Actually do the matching
         *
         * @param[in] filename name of event file to check
         */
        bool match(const std::string& filename ) const override {

            TFile *f = TFile::Open( filename.c_str() );
            if (!f) return false;
        
            TTreeReader r( "LDMX_Events" , f );
        
            //TODO check if tree loaded
            
            if ( r.GetEntries(true) != entries_ ) { f->Close(); return false; }

            //Event tree should _always_ have the EventHeader
            TTreeReaderValue<EventHeader> header( r , "EventHeader" );
        
            if ( checkCollection_ ) {
                TTreeReaderValue<std::vector<CalorimeterHit>> collection( r , ("TestCollection_"+pass_).c_str() );
                while ( r.Next() ) {
                    if ( collection->size() != header->getEventNumber() ) { f->Close(); return false; }
                    for ( unsigned int i = 0; i < collection->size(); i++ )
                        if ( collection->at(i).getID() != header->getEventNumber()*10+i ) { f->Close(); return false; }
                }
                //restart in case checking object as well
                r.Restart();
            }

            if ( checkObject_ ) {
                TTreeReaderValue<HcalVetoResult> object( r , ("TestObject_"+pass_).c_str() );
                while( r.Next() ) {
                    if ( object->getMaxPEHit().getID() != header->getEventNumber() ) { f->Close(); return false; }
                }
            }
             
            f->Close();
            
            return true;
        }

        /**
         * Human-readable true-fact statement for any match that is true.
         */
        virtual std::string describe() const override {
            std::ostringstream ss;
            ss << "can be opened and has the correct number of entries in the event tree.";
            if ( checkCollection_ ) ss << " TestCollection_" << pass_ << " was verified."; 
            if ( checkObject_     ) ss << " TestObject_"     << pass_ << " was verified."; 
            return ss.str();
        }

}; //isGoodEventFile

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

        const char *event_file_path = "test_productionmode_events.root";
        p.setOutputFileName( event_file_path );
        p.setEventLimit( 3 );
        p.addToSequence( proHdl );

        SECTION( "only producers" ) {
            CHECK_NOTHROW( p.run() );
        }
        
        SECTION( "with Analyses" ) {
            const char *hist_file_path = "test_productionmode_withanalyses_hists.root";
            p.setHistogramFileName( hist_file_path );
            p.addToSequence( anaHdl );
            CHECK_NOTHROW( p.run() );
            CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+3 ) );
            CHECK( remove( hist_file_path ) == 0 );
        }

        CHECK_THAT( event_file_path , isGoodEventFile( "test" , 3 ) );
        CHECK( remove( event_file_path ) == 0 );
    }//Production Mode

    SECTION( "Need Input Files" ) {

        Process makeInputs( "makeInputs" );
        TestProducer inputPro( makeInputs );
        auto inputProHdl = &inputPro;
        makeInputs.addToSequence( inputProHdl );
    
        const char * input_file_2_events = "test_needinputfiles_2_events.root";
        makeInputs.setOutputFileName( input_file_2_events );
        makeInputs.setEventLimit( 2 );
        REQUIRE_NOTHROW( makeInputs.run() );
        REQUIRE_THAT( input_file_2_events , isGoodEventFile( "makeInputs" , 2 ) );
    
        const char * input_file_3_events = "test_needinputfiles_3_events.root";
        makeInputs.setOutputFileName( input_file_3_events );
        makeInputs.setEventLimit( 3 );
        REQUIRE_NOTHROW( makeInputs.run() );
        REQUIRE_THAT( input_file_3_events , isGoodEventFile( "makeInputs" , 3 ) );
       
        const char * input_file_4_events = "test_needinputfiles_4_events.root";
        makeInputs.setOutputFileName( input_file_4_events );
        makeInputs.setEventLimit( 4 );
        REQUIRE_NOTHROW( makeInputs.run() );
        REQUIRE_THAT( input_file_4_events , isGoodEventFile( "makeInputs" , 4 ) );
    
        SECTION( "Analysis Mode" ) {
            //no output files, only histogram output

            p.addToSequence( anaHdl );

            const char *hist_file_path = "test_analysismode_hists.root";
            p.setHistogramFileName( hist_file_path );

            SECTION( "one input file" ) {
                p.addFileToProcess( input_file_2_events );
                p.run();

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2 ) );
                CHECK( remove( hist_file_path ) == 0 );
            }
            
            SECTION( "multiple input files" ) {
                p.addFileToProcess( input_file_2_events );
                p.addFileToProcess( input_file_3_events );
                p.addFileToProcess( input_file_4_events );
                p.run();

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+1+2+3+1+2+3+4 ) );
                CHECK( remove( hist_file_path ) == 0 );
            }

        }//Analysis Mode

        SECTION( "Merge Mode" ) {
            //many input files to one output file
            
            p.addFileToProcess( input_file_2_events );
            p.addFileToProcess( input_file_3_events );
            p.addFileToProcess( input_file_4_events );

            const char *event_file_path = "test_mergemode_events.root";
            p.setOutputFileName( event_file_path );

            SECTION( "no processors" ) {
                p.run();
            }
    
            SECTION( "with analyzers" ) {

                p.addToSequence( anaHdl );

                const char *hist_file_path = "test_mergemode_withanalyzers_hists.root";
                p.setHistogramFileName( hist_file_path );
    
                p.run();

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+1+2+3+1+2+3+4 ) );
                CHECK( remove( hist_file_path ) == 0 );
            }
    
            SECTION( "with producers" ) {
                p.addToSequence( proHdl );
                p.run();

                CHECK_THAT( event_file_path , isGoodEventFile( "test" , 2+3+4 ) );
            }

            //checks that input collections were copied over correctly
            CHECK_THAT( event_file_path , isGoodEventFile( "makeInputs" , 2+3+4 ) );
            CHECK( remove( event_file_path ) == 0 );

        } //Merge Mode

        SECTION( "N-to-N Mode" ) {
            //many input files to many output files
            p.addFileToProcess( input_file_2_events );
            p.addFileToProcess( input_file_3_events );
            p.addFileToProcess( input_file_4_events );

            const char * output_2_events = "test_NtoNmode_output_2_events.root";
            const char * output_3_events = "test_NtoNmode_output_3_events.root";
            const char * output_4_events = "test_NtoNmode_output_4_events.root";
            p.addOutputFileName( output_2_events );
            p.addOutputFileName( output_3_events );
            p.addOutputFileName( output_4_events );

            SECTION( "no processors" ) {
                p.run();
            }

            SECTION( "with analyzer" ) {
                p.addToSequence( anaHdl );

                const char *hist_file_path = "test_NtoNmode_withanalyzer_hists.root";
                p.setHistogramFileName( hist_file_path );
    
                p.run();

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+1+2+3+1+2+3+4 ) );
                CHECK( remove( hist_file_path ) == 0 );
            }
    
            SECTION( "with producers" ) {
                p.addToSequence( proHdl );
                p.run();

                //checks that produced objects were written correctly
                CHECK_THAT( output_2_events , isGoodEventFile( "test" , 2 ) );
                CHECK_THAT( output_3_events , isGoodEventFile( "test" , 3 ) );
                CHECK_THAT( output_4_events , isGoodEventFile( "test" , 4 ) );
            }

            //checks that input pass was copied over correctly
            CHECK_THAT( output_2_events , isGoodEventFile( "makeInputs" , 2 ) );
            CHECK( remove( output_2_events ) == 0 );

            CHECK_THAT( output_3_events , isGoodEventFile( "makeInputs" , 3 ) );
            CHECK( remove( output_3_events ) == 0 );

            CHECK_THAT( output_4_events , isGoodEventFile( "makeInputs" , 4 ) );
            CHECK( remove( output_4_events ) == 0 );

        } // N-to-N Mode

        //cleanup
        //  delete any files that this test produced
        //  test_out.root, test_input{0,1,2}.root, test_merge.root, test_histo.root
        CHECK( remove( input_file_2_events ) == 0 );
        CHECK( remove( input_file_3_events ) == 0 );
        CHECK( remove( input_file_4_events ) == 0 );

    } //need input files


}//process test
