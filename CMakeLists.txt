cmake_minimum_required(VERSION 3.12)

# make sure the source and binary directories are different
# this is here to prevent so-called "in-source" builds where the root directory of the source
# is also the root directory of the build
get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)
if ("${srcdir}" STREQUAL "${bindir}")
  message(FATAL_ERROR "ldmx-sw does not support in-source builds.
  Call 'denv cmake ..' from within a 'build/' subdirectory or
  Tell cmake to use a different build directory with 'denv cmake -B build -S .'
  'just configure' and 'just build' will do the necessary directory movement for you as well.")
endif()

# Set the project name and version
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION LDMXSW_VERSION)
string(STRIP "${LDMXSW_VERSION}" LDMXSW_VERSION)
string(REGEX REPLACE "^v" "" LDMXSW_VERSION_NUM "${LDMXSW_VERSION}") 

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/COMMIT_SHA COMMIT_SHA)
string(STRIP "${COMMIT_SHA}" COMMIT_SHA)

project(LDMX_SW VERSION ${LDMXSW_VERSION_NUM} 
                DESCRIPTION "The Light Dark Matter eXperiment simulation and reconstruction framework." 
                LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_MESSAGE LAZY)

# Load additional macros used by this project. 
list(APPEND CMAKE_MODULE_PATH ${LDMX_SW_SOURCE_DIR}/cmake/)

# Load the BuildMacros module. If loaded correctly, the variable 
# 'build_macros_found' will be set to the path of the module.  Otherwise, it
# is set to NOTFOUND.
include(BuildMacros RESULT_VARIABLE build_macros_found)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Generate the ROOT dictionary.  The following allows the use of the macro used
# to generate the dictionary.
include("${ROOT_DIR}/RootMacros.cmake")

# Adding ROOT as "SYSTEM" 
# which will silence compiler warnings from these 3rd party softwares
include_directories(SYSTEM /usr/local/include/root)

# Search for the Python3 library
find_package(Python3 COMPONENTS Interpreter Development)

# CMake target wrapper around Geant4 CMake configuration
find_package(Geant4Interface)

find_package(GENIE)

find_package(
  Boost REQUIRED
  COMPONENTS system
             log
             filesystem
             thread
             chrono
             atomic
             regex)


# FunctionalCoreTest.cxx doesnt comply with clang-tidy but that's ok
set_source_files_properties(Framework/test/FunctionalCoreTest.cxx PROPERTIES COMPILE_OPTIONS "-Wno-null-dereference")

# If an install location hasn't been set via CMAKE_INSTALL_PREFIX, set it to 
# a reasonable default ($PWD/install).  
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install CACHE PATH "" FORCE)
    message(STATUS "Install directory set to ${CMAKE_INSTALL_PREFIX}")
endif()

# Set the default release type to "Release". If a release type is specified 
# at the command line, it's respected.
set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()

# Clear any variables cached during previous configuration cycles. 
clear_cache_variables()

add_subdirectory(Framework)
add_subdirectory(DetDescr)
add_subdirectory(Packing)
add_subdirectory(Conditions)
add_subdirectory(Tools)
add_subdirectory(Ecal)
add_subdirectory(Recon)
add_subdirectory(DQM)
add_subdirectory(Hcal)
add_subdirectory(TrigScint)
add_subdirectory(Tracking)
add_subdirectory(Trigger/Algo)
add_subdirectory(SimCore)
add_subdirectory(Biasing)
add_subdirectory(Detectors)
add_subdirectory(MagFieldMap)

build_test()
add_test(
  NAME PythonConfigParameterSet
  COMMAND python3 -m unittest Framework/test/test_parameter_set.py
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_test(
  NAME StandaloneProcessor
  COMMAND ./run
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Framework/test/standalone
)
