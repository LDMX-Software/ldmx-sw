#include "Exception/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/ConfigurePython.h" //headers defining what we will be testing
#include "Framework/Process.h"
#include "Framework/EventProcessor.h"

#include <fstream>

namespace ldmx { namespace test {

    /**
     * @class TestConfig
     * @brief Defines a dummy Producer to test the passing of configuration variables
     */
    class TestConfig : public Producer {

        public:

            /**
             * Constructor
             *
             * Follows the standard form for a ldmx::Producer.
             *
             * Checks that the passed name is the same as
             * what is written to the python config script.
             */
            TestConfig(const std::string& name, Process& process) :
                Producer(name, process) {

                    CHECK( name == "testInstance" );
            }

            /**
             * Configure function
             *
             * Checks:
             * - int parameter
             * - double parameter
             * - string parameter
             * - vector of ints parameter
             * - vector of doubles parameter
             * - vector of strings parameter
             */ 
            void configure(Parameters& parameters) final override {
            
                CHECK( parameters.getParameter< int >("testInt") == 9 );
                CHECK( parameters.getParameter< double >("testDouble") == Approx( 7.7 ) );
                CHECK( parameters.getParameter< std::string >("testString") == "Yay!" );

                std::vector<int> correctIntVec = { 1 , 2 , 3 };
                auto testIntVec{parameters.getParameter<std::vector<int>>("testIntVec")};
                REQUIRE( testIntVec.size() == correctIntVec.size() );
                for(std::size_t i{0}; i < testIntVec.size(); i++) CHECK( testIntVec.at(i) == correctIntVec.at(i) );

                std::vector<double> correctDoubleVec = { 0.1 , 0.2 , 0.3 };
                auto testDoubleVec{parameters.getParameter<std::vector<double>>("testDoubleVec")};
                REQUIRE( testDoubleVec.size() == correctDoubleVec.size() );
                for(std::size_t i{0}; i < testDoubleVec.size(); i++) CHECK( testDoubleVec.at(i) == correctDoubleVec.at(i) );

                std::vector<std::string> correctStringVec = { "first" , "second" , "third" };
                auto testStringVec{parameters.getParameter<std::vector<std::string>>("testStringVec")};
                REQUIRE( testStringVec.size() == correctStringVec.size() );
                for(std::size_t i{0}; i < testStringVec.size(); i++) CHECK( testStringVec.at(i) == correctStringVec.at(i) );
            }

            /**
             * I don't do anything.
             */
            virtual void produce(Event&) { }

    };


/**
 * @func removeFile
 * Deletes the file and returns whether the deletion was successful.
 *
 * This is just a helper function during development.
 * Sometimes it is helpful to leave the generated files, so
 * maybe we can make the removal optional?
 */
static bool removeFile(const char * filepath) {
    return remove( filepath ) == 0;
}

} //test
} //ldmx

DECLARE_PRODUCER_NS(ldmx::test, TestConfig)

/**
 * Test for Configure Python class
 *
 * Checks:
 * - pass parameters to Process object
 * - pass parameters to EventProcessors
 * - pass parameters to python script on command line
 * - TODO pass histogram info to EventProcessors
 * - TODO pass class objects to EventProcessors
 */
TEST_CASE( "Configure Python Test" , "[Framework][functionality]" ) {

#if PY_MAJOR_VERSION < 3
    std::cerr << "Python2 doesn't play nice with Catch!" << std::endl;
    CHECK(false);
#else
    const std::string configFileName{"test_config_script.py"};
    
    std::ofstream testPyScript( configFileName );

    testPyScript << "from LDMX.Framework import ldmxcfg" << std::endl;

    testPyScript << "testProcess    = ldmxcfg.Process( 'test' )" << std::endl;
    testPyScript << "testProcess.inputFiles = [ 'input1' , 'input2' ]" << std::endl;
    testPyScript << "testProcess.logFrequency = 9" << std::endl;
    testPyScript << "testProcess.skimDefaultIsKeep = False" << std::endl;

    testPyScript << "testProcessor = ldmxcfg.Producer( 'testInstance' , 'ldmx::test::TestConfig' )" << std::endl;
    testPyScript << "testProcessor.parameters['testInt'] = 9" << std::endl;
    testPyScript << "testProcessor.parameters['testDouble'] = 7.7" << std::endl;
    testPyScript << "testProcessor.parameters['testString'] = 'Yay!'" << std::endl;
    testPyScript << "testProcessor.parameters['testIntVec'] = [ 1 , 2 , 3 ]" << std::endl;
    testPyScript << "testProcessor.parameters['testDoubleVec'] = [ 0.1 , 0.2 , 0.3 ]" << std::endl;
    testPyScript << "testProcessor.parameters['testStringVec'] = [ 'first' , 'second' , 'third' ]" << std::endl;

    testPyScript << "testProcess.sequence = [ testProcessor ]" << std::endl;

    using namespace ldmx;

    char *args[1];

    ProcessHandle p;

    int correctLogFreq{9};

    /*
     * Python seg faults when trying to run more than once.
     * Both this SECTION and the un-commented SECTION
     * run fine alone, but a segfault is produced
     * upon starting the next section.
     *
     * I'm not sure how to get around this, so I am
     * just going to leave the more rigorous test in place.
    SECTION( "no arguments to python script" ) {

        testPyScript.close();

        ConfigurePython cfg( configFileName , args , 0 );

        p = cfg.makeProcess();
    }
     */

    SECTION( "one argument to python script" ) {

        testPyScript << "import sys" << std::endl;
        testPyScript << "testProcess.logFrequency = int(sys.argv[1])" << std::endl;
        testPyScript.close();

        args[0] = "9000";
        correctLogFreq = 9000;

        ConfigurePython cfg( configFileName , args , 1 );

        p = cfg.makeProcess();
    }

    CHECK( p->getPassName() == "test" );
    CHECK( p->getLogFrequency() == correctLogFreq );

    CHECK( test::removeFile( configFileName.c_str() ) );
#endif
}
