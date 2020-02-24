/**
 * @file catch.cxx
 * @brief File to implement unit testing program.
 * @author Tom Eichlersmith, University of Minnesota
 */

/**
 * This define statement tells the Catch2 code to pull together
 * all of the macros throughout ldmx-sw and compile them into a
 * single executable here.
 *
 * Check out the Catch2 [documentation](https://github.com/catchorg/Catch2/blob/master/docs/tutorial.md#top) to learn more about how to write a test.
 *
 * There can only be *one* instance of this definition.
 * I have chosen to put it here for now.
 */
#define CATCH_CONFIG_MAIN
#include "Exception/catch.hpp"
