# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(TrigScint VERSION 2.1.0
                  DESCRIPTION "Module for all things trigger scintillator."
                  LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

  register_event_object( module_path "TrigScint/Event" namespace "ldmx" 
                         class "TrigScintHit" type "collection" )
  register_event_object( module_path "TrigScint/Event" namespace "trigscint" 
                         class "TrigScintQIEDigis" type "collection" )
  register_event_object( module_path "TrigScint/Event" namespace "ldmx" 
                         class "TrigScintCluster" type "collection" )
  register_event_object( module_path "TrigScint/Event" namespace "ldmx" 
                         class "TrigScintTrack" type "collection" )
  
  # Generate the files needed to build the event classes.
  setup_library(module TrigScint name Event
                dependencies ROOT::Core
                             Hcal::Event
			     TrigScint::Event
                             Recon::Event
                register_target)

  return()

endif()

setup_library(module TrigScint
              dependencies Framework::Framework Recon::Event DetDescr::DetDescr
                           Tools::Tools SimCore::Event
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/TrigScint)

