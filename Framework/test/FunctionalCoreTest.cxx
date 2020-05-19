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
#include "Framework/EventFile.h"

using namespace ldmx;

/**
 * @class TestProducer
 * Bare producer that creates a collection and an object and puts them 
 * on the event bus.
 *
 * The pattern this producer creates is the following:
 * - The vector of Calorimeter hits has the same number of entries as the event number
 * - The IDs of the calorimeter hits are set to 10*eventNumber+their_index
 * - The input object is an HcalVetoResult where events with an event index pass
 * - The max PE hit in the HcalVetoResult has an ID equal to the event index
 * - If a run header is created, the event count and the run number are equal
 *
 * Checks 
 * - Event::add function does not throw any errors.
 * - Writes and adds a run header where the run number and the number of events are the same.
 * - sets a storage hint
 */
class TestProducer : public Producer {

        /// number of events we've gotten to
        int events_;

        /// should we create the run header?
        bool createRunHeader_{false};

    public:

        TestProducer(Process& p) : Producer( "TestProducer" , p ) { }
        ~TestProducer() { }

        void shouldMakeRunHeader(bool yes) { createRunHeader_ = yes; }

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

            events_ = i_event;

            if ( res.passesVeto() ) setStorageHint( hint_mustKeep );

            return;
        }

        void onFileClose(EventFile& eventFile) final override {

            if ( not createRunHeader_ ) return;

            RunHeader runHeader( events_ , "No Detector" , "Test Run" );
            runHeader.setIntParameter( "Event Count" , events_ );

            //stores run header to runtree in output file
            eventFile.writeRunHeader( runHeader );

            return;
        }
};//TestProducer

/**
 * @class TestAnalyzer
 * Bare analyzer that looks for objects matching what the TestProducer put in.
 *
 * Checks
 * - the correct number and contents following the pattern produced by TestProducer.
 * - Event::getCollection and Event::getObject don't throw errors.
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
 * - The histogram has the correct number of entries
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
 * Checks:
 * - Event File exists and is readable
 * - Has the LDMX_Events TTree
 * - Events tree has correct number of entries
 * - if existCollection: looks through collections on the event tree to make sure they have the same form as set by TestProducer
 * - if existObject: looks through objects on event tree to make sure they have the same form as set by TestProducer
 * - Has the LDMX_Run TTree
 * - Run tree has correct number of entries
 * - RunHeaders in RunTree have matching RunNumbers and EventCounts
 */
class isGoodEventFile : public Catch::MatcherBase<std::string> {
    private:
        /// pass name to check the collection and/or object for
        std::string pass_;

        /// correct number of entries in the event ttree
        int entries_;

        /// correct number of runs
        int runs_;

        /// collection should exist in file
        bool existCollection_;

        /// object should exist in file
        bool existObject_;

    public:

        /**
         * Constructor
         *
         * Sets the correct number of entries and the other checking parameters
         */
        isGoodEventFile( 
                const std::string& pass , const int& entries , const int& runs , bool existColl = true , bool existObj = true )
            : pass_(pass), entries_(entries), runs_(runs), existCollection_(existColl), existObject_(existObj) { }

        /**
         * Actually do the matching
         *
         * The event and run tree names are hardcoded.
         * The branchnames are also hardcoded.
         *
         * @param[in] filename name of event file to check
         */
        bool match(const std::string& filename ) const override {

            TFile *f = TFile::Open( filename.c_str() );
            if (!f) return false;
        
            TTreeReader events( "LDMX_Events" , f );
        
            if ( events.GetEntries(true) != entries_ ) { f->Close(); return false; }

            //Event tree should _always_ have the EventHeader
            TTreeReaderValue<EventHeader> header( events , "EventHeader" );
        
            if ( existCollection_ ) {
                //make sure collection matches pattern
                TTreeReaderValue<std::vector<CalorimeterHit>> collection( events , ("TestCollection_"+pass_).c_str() );
                while ( events.Next() ) {
                    if ( collection->size() != header->getEventNumber() ) { f->Close(); return false; }
                    for ( unsigned int i = 0; i < collection->size(); i++ )
                        if ( collection->at(i).getID() != header->getEventNumber()*10+i ) { f->Close(); return false; }
                }
                //restart in case checking object as well
                events.Restart();
            } else {
                //check to make sure collection is NOT there
                auto t{(TTree*)f->Get("LDMX_Events")};
                if (t and t->GetBranch( ("TestCollection_"+pass_).c_str() ) ) { f->Close(); return false; }
            }

            if ( existObject_ ) {
                //make sure object matches pattern
                TTreeReaderValue<HcalVetoResult> object( events , ("TestObject_"+pass_).c_str() );
                while( events.Next() ) {
                    if ( object->getMaxPEHit().getID() != header->getEventNumber() ) { f->Close(); return false; }
                }
            } else {
                //check to make sure object is NOT there
                auto t{(TTree*)f->Get("LDMX_Events")};
                if (t and t->GetBranch( ("TestObject_"+pass_).c_str() ) ) { f->Close(); return false; }
            }

            TTreeReader runs( "LDMX_Run" , f );

            if ( runs.GetEntries(true) != runs_ ) { f->Close(); return false; }

            TTreeReaderValue<RunHeader> runHeader( runs , "RunHeader" );

            while ( runs.Next() ) {
                if ( runHeader->getRunNumber() != runHeader->getIntParameter( "Event Count" ) ) { f->Close(); return false; }
            }
             
            f->Close();
            
            return true;
        }

        /**
         * Human-readable statement for any match that is true.
         */
        virtual std::string describe() const override {
            std::ostringstream ss;
            ss << "can be opened and has the correct number of entries in the event tree and the run tree.";

            ss << " TestCollection_" << pass_ << " was verified to "; 
            if ( existCollection_ ) ss << " be the correct pattern.";
            else                    ss << " not be in the file.";

            ss << " TestObject_" << pass_ << " was verified to "; 
            if ( existObject_ ) ss << " be the correct pattern.";
            else                ss << " not be in the file.";

            return ss.str();
        }

}; //isGoodEventFile

/**
 * @func removeFile
 * Deletes the file and returns whether the deletion was successful.
 *
 * This is just a helper function during development.
 * Sometimes it is helpful to leave the generated files, so
 * maybe we can make the removal optional?
 */
bool removeFile(const char * filepath) {
    return remove( filepath ) == 0;
}

/**
 * Test for C++ Framework processing.
 *
 * This test is aimed at checking that core functionalities are operational.
 * Python configuration, Simulation, and other add-on functionalities
 * are tested separately.
 *
 * This test does not check complicated combinations for drop/keep rules and skimming rules. 
 * I (Tom E) avoided this because this test is already very large and complicated.
 * TODO write a more full (and separate) test for these parts of the framework.
 *
 * Assumptions:
 *  - Any vector of objects behaves like a vector of CalorimeterHits when viewed from core
 *  - Any object behaves like a HcalVetoResult when viewed from core 
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
 *  - writing to an output file
 *  - writing and reading run headers
 *  - drop/keep rules for event bus passengers
 *  - skimming events (only keeping events meeting a certain criteria)
 */
TEST_CASE( "Core Framework Functionality" , "[Framework][functionality]" ) {

    Process p("test");

    REQUIRE( p.getPassName() == "test" );

    //Process owns and deletes the processors
    auto proHdl = new TestProducer( p );
    auto anaHdl = new TestAnalyzer( p );

    SECTION( "Production Mode" ) {
        //no input files, only output files

        const char *event_file_path = "test_productionmode_events.root";
        p.setOutputFileName( event_file_path );
        p.setEventLimit( 3 );
        p.addToSequence( proHdl );

        proHdl->shouldMakeRunHeader( true );
        p.setRunNumber( 3 );

        SECTION( "only producers" ) {
            
            SECTION( "no drop/keep rules" ) {
                p.run();
                CHECK_THAT( event_file_path , isGoodEventFile( "test" , 3 , 1 ) );
            }
            
            SECTION( "drop TestCollection" ) {
                p.addDropKeepRule( "drop .*Collection.*" );
                p.run();
                CHECK_THAT( event_file_path , isGoodEventFile( "test" , 3 , 1 , false ) );
            }

            SECTION( "skim for even indexed events" ) {
                p.getStorageController().setDefaultKeep( false );
                p.getStorageController().addRule( "TestProducer" , "" );
                p.run();
                CHECK_THAT( event_file_path , isGoodEventFile( "test" , 1 , 1 ) );
            }

        }
        
        SECTION( "with Analyses" ) {
            const char *hist_file_path = "test_productionmode_withanalyses_hists.root";
            p.setHistogramFileName( hist_file_path );
            p.addToSequence( anaHdl );

            SECTION( "no drop/keep rules" ) {
                p.run();
                CHECK_THAT( event_file_path , isGoodEventFile( "test" , 3 , 1 ) );
            }

            SECTION( "drop TestCollection" ) {
                p.addDropKeepRule( "drop .*Collection.*" );
                p.run();
                CHECK_THAT( event_file_path , isGoodEventFile( "test" , 3 , 1 , false ) );
            }

            SECTION( "skim for even indexed events" ) {
                p.getStorageController().setDefaultKeep( false );
                p.getStorageController().addRule( "TestProducer" , "" );
                p.run();
                CHECK_THAT( event_file_path , isGoodEventFile( "test" , 1 , 1 ) );
            }

            CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+3 ) );
            CHECK( removeFile( hist_file_path ) );
        }

        CHECK( removeFile( event_file_path ) );
    }//Production Mode

    SECTION( "Need Input Files" ) {

        Process makeInputs( "makeInputs" );
        auto inputProHdl = new TestProducer( makeInputs );
        makeInputs.addToSequence( inputProHdl );
        inputProHdl->shouldMakeRunHeader( true );
    
        const char * input_file_2_events = "test_needinputfiles_2_events.root";
        makeInputs.setOutputFileName( input_file_2_events );
        makeInputs.setEventLimit( 2 );
        makeInputs.setRunNumber( 2 );
        REQUIRE_NOTHROW( makeInputs.run() );
        REQUIRE_THAT( input_file_2_events , isGoodEventFile( "makeInputs" , 2 , 1) );
    
        const char * input_file_3_events = "test_needinputfiles_3_events.root";
        makeInputs.setOutputFileName( input_file_3_events );
        makeInputs.setEventLimit( 3 );
        makeInputs.setRunNumber( 3 );
        REQUIRE_NOTHROW( makeInputs.run() );
        REQUIRE_THAT( input_file_3_events , isGoodEventFile( "makeInputs" , 3 , 1) );
       
        const char * input_file_4_events = "test_needinputfiles_4_events.root";
        makeInputs.setOutputFileName( input_file_4_events );
        makeInputs.setEventLimit( 4 );
        makeInputs.setRunNumber( 4 );
        REQUIRE_NOTHROW( makeInputs.run() );
        REQUIRE_THAT( input_file_4_events , isGoodEventFile( "makeInputs" , 4 , 1) );
    
        SECTION( "Analysis Mode" ) {
            //no output files, only histogram output

            p.addToSequence( anaHdl );

            const char *hist_file_path = "test_analysismode_hists.root";
            p.setHistogramFileName( hist_file_path );

            SECTION( "one input file" ) {
                p.addFileToProcess( input_file_2_events );

                p.run();

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2 ) );
                CHECK( removeFile( hist_file_path ) );
            }
            
            SECTION( "multiple input files" ) {
                p.addFileToProcess( input_file_2_events );
                p.addFileToProcess( input_file_3_events );
                p.addFileToProcess( input_file_4_events );
                p.run();

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+1+2+3+1+2+3+4 ) );
                CHECK( removeFile( hist_file_path ) );
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

                SECTION( "no drop/keep rules" ) {
                    p.run();
                    CHECK_THAT( event_file_path , isGoodEventFile( "makeInputs" , 2+3+4 , 3 ) );
                }

                SECTION( "drop TestCollection" ) {
                    p.addDropKeepRule( "drop .*Collection.*" );
                    p.run();
                    CHECK_THAT( event_file_path , isGoodEventFile( "makeInputs" , 2+3+4 , 3 , false ) );
                }
            }
    
            SECTION( "with analyzers" ) {

                p.addToSequence( anaHdl );

                const char *hist_file_path = "test_mergemode_withanalyzers_hists.root";
                p.setHistogramFileName( hist_file_path );
    
                SECTION( "no drop/keep rules" ) {
                    p.run();
                    CHECK_THAT( event_file_path , isGoodEventFile( "makeInputs" , 2+3+4 , 3 ) );
                }

                SECTION( "drop TestCollection" ) {
                    p.addDropKeepRule( "drop .*Collection.*" );
                    p.run();
                    CHECK_THAT( event_file_path , isGoodEventFile( "makeInputs" , 2+3+4 , 3 , false ) );
                }

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+1+2+3+1+2+3+4 ) );
                CHECK( removeFile( hist_file_path ) );
            }
    
            SECTION( "with producers" ) {
                p.addToSequence( proHdl );

                SECTION( "not listening to storage hints" ) {
                    p.run();
                    CHECK_THAT( event_file_path , isGoodEventFile( "test" , 2+3+4 , 3 ) );
                }

                SECTION( "skim for even indexed events" ) {
                    p.getStorageController().setDefaultKeep( false );
                    p.getStorageController().addRule( "TestProducer" , "" );
                    p.run();
                    CHECK_THAT( event_file_path , isGoodEventFile( "test" , 1+1+2 , 3 ) );
                }

            }

            CHECK( removeFile( event_file_path ) );

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
                
                SECTION( "no drop/keep rules" ) {
                    p.run();
                    CHECK_THAT( output_2_events , isGoodEventFile( "makeInputs" , 2 , 1 ) );
                    CHECK_THAT( output_3_events , isGoodEventFile( "makeInputs" , 3 , 1 ) );
                    CHECK_THAT( output_4_events , isGoodEventFile( "makeInputs" , 4 , 1 ) );
                }

                SECTION( "drop TestCollection" ) {
                    p.addDropKeepRule( "drop .*Collection.*" );
                    p.run();
                    CHECK_THAT( output_2_events , isGoodEventFile( "makeInputs" , 2 , 1 , false ) );
                    CHECK_THAT( output_3_events , isGoodEventFile( "makeInputs" , 3 , 1 , false ) );
                    CHECK_THAT( output_4_events , isGoodEventFile( "makeInputs" , 4 , 1 , false ) );
                }
            }

            SECTION( "with analyzer" ) {
                p.addToSequence( anaHdl );

                const char *hist_file_path = "test_NtoNmode_withanalyzer_hists.root";
                p.setHistogramFileName( hist_file_path );
    
                SECTION( "no drop/keep rules" ) {
                    p.run();
                    CHECK_THAT( output_2_events , isGoodEventFile( "makeInputs" , 2 , 1 ) );
                    CHECK_THAT( output_3_events , isGoodEventFile( "makeInputs" , 3 , 1 ) );
                    CHECK_THAT( output_4_events , isGoodEventFile( "makeInputs" , 4 , 1 ) );
                }

                SECTION( "drop TestCollection" ) {
                    p.addDropKeepRule( "drop .*Collection.*" );
                    p.run();
                    CHECK_THAT( output_2_events , isGoodEventFile( "makeInputs" , 2 , 1 , false ) );
                    CHECK_THAT( output_3_events , isGoodEventFile( "makeInputs" , 3 , 1 , false ) );
                    CHECK_THAT( output_4_events , isGoodEventFile( "makeInputs" , 4 , 1 , false ) );
                }

                CHECK_THAT( hist_file_path , isGoodHistogramFile( 1+2+1+2+3+1+2+3+4 ) );
                CHECK( removeFile( hist_file_path ) );
            }
    
            SECTION( "with producers" ) {
                p.addToSequence( proHdl );

                SECTION( "not listening to storage hints" ) {
                    p.run();
    
                    //checks that produced objects were written correctly
                    CHECK_THAT( output_2_events , isGoodEventFile( "test" , 2 , 1 ) );
                    CHECK_THAT( output_3_events , isGoodEventFile( "test" , 3 , 1 ) );
                    CHECK_THAT( output_4_events , isGoodEventFile( "test" , 4 , 1 ) );
                }

                SECTION( "skimming for even-indexed events" ) {
                    p.getStorageController().setDefaultKeep( false );
                    p.getStorageController().addRule( "TestProducer" , "" );
                    p.run();
                    CHECK_THAT( output_2_events , isGoodEventFile( "test" , 1 , 1 ) );
                    CHECK_THAT( output_3_events , isGoodEventFile( "test" , 1 , 1 ) );
                    CHECK_THAT( output_4_events , isGoodEventFile( "test" , 2 , 1 ) );
                }
            }

            CHECK( removeFile( output_2_events ) );
            CHECK( removeFile( output_3_events ) );
            CHECK( removeFile( output_4_events ) );

        } // N-to-N Mode

        //cleanup
        CHECK( removeFile( input_file_2_events ) );
        CHECK( removeFile( input_file_3_events ) );
        CHECK( removeFile( input_file_4_events ) );

    } //need input files

}//process test
