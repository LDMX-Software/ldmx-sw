# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Trigger VERSION 2.1.0
                  DESCRIPTION "Module for trigger algorithms."
                  LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

  register_event_object( module_path "Trigger/Event" namespace "ldmx" 
                         class "TrigEnergySum" type "collection" )
  
  # Generate the files needed to build the event classes.
  setup_library(module Trigger name Event
                dependencies ROOT::Core
                             Recon::Event
                register_target)

  return()

endif()


include_directories(../submodules/HLS_arbitrary_Precision_Types/include)

setup_library(module Trigger
              dependencies Framework::Framework Recon::Event DetDescr::DetDescr
                           Tools::Tools SimCore::Event Ecal::Ecal Hcal::Hcal
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Trigger)
