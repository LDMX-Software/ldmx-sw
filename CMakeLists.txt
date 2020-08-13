
# Set the minimum version of CMake that's required 
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(LDMX_SW VERSION 2.1.0
                DESCRIPTION "The Light Dark Matter eXperiment simulation and reconstruction framework." 
                LANGUAGES CXX    
)

# If an install location hasn't been set via CMAKE_INSTALL_PREFIX, set it to 
# a reasonable default ($PWD/install).  
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install CACHE PATH "" FORCE)
    message(STATUS "Install directory set to ${CMAKE_INSTALL_PREFIX}")
endif()

# Load additional macro commands 
list(APPEND CMAKE_MODULE_PATH ${LDMX_SW_SOURCE_DIR}/cmake/)

# Load the BuildMacros module. If loaded correctly, the variable 
# 'build_macros_found' will be set to the path of the module.  Otherwise, it
# is set to NOTFOUND.
include(BuildMacros RESULT_VARIABLE build_macros_found)

# If a user is building outside of a Docker or Singularity environment, 
# warn them. 
if (NOT EXISTS /.dockerenv AND NOT EXISTS /singularity)
    message(WARNING "You are not inside a container; you may be working in an untested environment.")
endif()

# Set the python installation path
set(PYTHON_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/python/LDMX)
message(STATUS "Python install directory set to ${PYTHON_INSTALL_PREFIX}")

# Set the default release type to "Release". If a release type is specified 
# at the command line, it's respected.
set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()

# Add options to allow the following types of builds
#
# * Recon - In addition to building the framework, build the modules needed 
# by the reconstruction.  
# * Sim - Only build the modules necessary to run the simulation.
# * Eve - Build the event display only.
#
# By default, all modules are built.

option(BUILD_RECON_ONLY     "Build the modules necessary to run the reconstruction." OFF)
option(BUILD_SIM_ONLY       "Build the modules necessary to run the simulation."     OFF)
option(BUILD_EVE_ONLY       "Build the event display only."                          OFF)

if(NOT BUILD_RECON_ONLY AND NOT BUILD_SIM_ONLY AND NOT BUILD_EVE_ONLY) 
    set(BUILD_ALL ON)
    message(STATUS "Building all modules.")
endif()

# This modules contains the event model classes.
add_subdirectory(Event)

# The framework has all the backend code needed to run all processors.
add_subdirectory(Framework)

# This module contains all detector service related code.
add_subdirectory(DetDescr)

if(BUILD_ALL OR BUILD_RECON_ONLY)

    message(STATUS "Building the modules necessary for the reconstruction.")

    # The tools module contains tools generic enough to be used by other modules.
    add_subdirectory(Tools)

    # This module contains ECal reconstruction code.
    add_subdirectory(Ecal)

    # Currently, this module contains processors from several subsystems.
    add_subdirectory(EventProc)

    # Data quality management plots.
    add_subdirectory(DQM)

endif()

if(BUILD_ALL OR BUILD_SIM_ONLY)

    message(STATUS "Building the modules necessary for the simulation.")

    # SimCore allows the simulation to be run without biasing
    add_subdirectory(SimCore)

    # Adding the Biasing module, allows the use of biasing in the simulation
    add_subdirectory(Biasing)

    # Add the Detectors submodule.  This has no dependecies.
    add_subdirectory(Detectors)

endif()

if(BUILD_EVE_ONLY)
    
    message(STATUS "Building the event display.")
    add_subdirectory(EventDisplay)

endif()

# Magnetic field map installation
option(INSTALL_FIELDMAP "Install field map data from MagFieldMap sub-module" ON)
if(INSTALL_FIELDMAP)
  install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/InstallMagFieldMap.cmake")
endif()

# If enabled, build all test.  The test are assumed to reside inside of the
# directory "test" within each module.  The main
option(BUILD_TESTS "Build and link the run-test executable." OFF)
if(BUILD_TESTS)
    build_test()
endif()

# install python init file for top-level LDMX module
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py 
    "\"\"\"Python package to configure ldmx-sw\"\"\"")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION lib/python/LDMX)

