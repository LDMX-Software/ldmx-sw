# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Trigger VERSION 0.1.0
                  DESCRIPTION "Module for trigger algorithms."
                  LANGUAGES CXX
)

setup_library(
  module Trigger
  name Event
  dependencies ROOT::Core Recon::Event
  linkdef include/Trigger/Event/LinkDef.h
)

setup_library(module Trigger
              dependencies Framework::Framework Recon::Event DetDescr::DetDescr
                           Tools::Tools SimCore::Event Ecal::Ecal Hcal::Hcal
                           Trigger::Event
)
target_include_directories(Trigger PUBLIC ../HLS_arbitrary_Precision_Types/include)

setup_python(package_name LDMX/Trigger)
