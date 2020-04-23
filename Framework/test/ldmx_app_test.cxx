/**
 * @file ldmx_app_test.cxx
 * @brief Test the operation of ldmx-app program
 *
 * @author Tom Eichlersmith, University of Minnesota
 */
#include "Exception/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/Process.h"
#include "Framework/ConfigurePython.h"

/**
 * Test for ldmx-app function
 *
 * First argument is name of this test.
 * Second argument is tags to group tests together.
 */
TEST_CASE( "ldmx-app Functionality Test" , "[Framework][functionality]" ) {

    //test python configuration
    //  run script
    //  read in parameters and give them to process and processors


    SECTION( "Production Mode" ) {
        //no input files, only output files
        
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
