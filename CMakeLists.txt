# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  SimCore
  VERSION 2.1.0
  DESCRIPTION "Core classes needed to run the Simulator."
  LANGUAGES CXX)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

# Search for Boost
find_package(Boost REQUIRED
             COMPONENTS log)

option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

  register_event_object(module_path "SimCore/Event" namespace "ldmx" 
                        class "SimCalorimeterHit" type "collection")
  register_event_object(module_path "SimCore/Event" namespace "ldmx"
                        class "SimTrackerHit" type "collection")
  register_event_object(module_path "SimCore/Event" namespace "ldmx"
                        class "SimParticle" type "map" key "int")

  # Get all of the event sources that will be included in this library.
  file(GLOB EVENT_SRC_FILES ${PROJECT_SOURCE_DIR}/src/SimCore/Event/*.cxx)

  # Generate the files needed to build the event classes.
  setup_library(module SimCore name Event
                dependencies ROOT::Core
                sources ${EVENT_SRC_FILES} register_target)

  return()

endif()

# Configure Geant4
setup_geant4_target()

# Get a list of all of the source files.
file(GLOB SRC_FILES CONFIGURE_DEPDENDS ${PROJECT_SOURCE_DIR}/src/SimCore/Persist/*.cxx
                                       ${PROJECT_SOURCE_DIR}/src/SimCore/*.cxx)

# Setup the library
setup_library(name SimCore
              dependencies Geant4::Interface
                           ROOT::Physics
                           Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr
                           Boost::log
                           "${registered_targets}"
              sources ${SRC_FILES})

# Set some target properties
set_target_properties(SimCore
                      PROPERTIES CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/SimCore)
