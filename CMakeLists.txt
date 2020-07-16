
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(SimCore VERSION 2.1.0
                DESCRIPTION "Core classes needed to run the Simulator."
                LANGUAGES CXX
)

# Check if the project specifc macros have been loaded. If not, and the 
# stand alone build option has been given, clone the project locally and 
# include them.  Otherwise, error out.
if (NOT build_macros_found)
    if (stand_alone_build)
        # Pull in the cmake module as an external package.
    else()
        message(FATAL_ERROR "Project specific build modules have not been loaded.")
    endif()
endif()

# Configure Geant4
setup_geant4_target()

# Setup the library 
setup_library(name SimCore 
              dependencies Geant4::Interface DARK::Framework DARK::DetDescr)

# Set some target properties
set_target_properties(SimCore PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
