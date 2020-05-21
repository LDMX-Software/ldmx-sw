/**
 * @file DetectorIDTest.cxx
 * @brief Test the operation of DetectorID class
 */
#include "Exception/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/ConfigurePython.h" //headers defining what we will be testing

using namespace ldmx;

/**
 * Test for Configure Python class
 *
 * Checks:
 * - TODO pass parameters to Process object
 * - TODO pass parameters to EventProcessors
 * - TODO pass histogram info to EventProcessors
 * - TODO pass class objects to EventProcessors
 */
TEST_CASE( "Configure Python Test" , "[Framework][functionality]" ) {
    
    std::ofstream testPyScript( "test_config_script.py" );

    testPyScript << "from LDMX.Framework import ldmxcfg" << std::endl;
    testPyScript << "testProcess    = ldmxcfg.Process( \"test\" )" << std::endl;
    testPyScript << "testProcess.inputFiles = [ \"input1\" , \"input2\" ]" << std::endl;
    testPyScript << "testProcess.logFrequency = 9" << std::endl;
    testPyScript << "testProcess.skimDefaultIsKeep = False" << std::endl;
    testPyScript << "testProccessor = ldmxcfg.Producer( \"testInstance\" , \"TestClassName\" )" << std::endl;
}
