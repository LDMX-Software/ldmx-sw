cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Tracking DESCRIPTION "Module containing the sim and recon code for the trackers."
                 LANGUAGES CXX
)

setup_library(
  module Tracking
  name Event
  dependencies ROOT::Core SimCore::Event
  linkdef include/Tracking/Event/LinkDef.h
)

find_package(Eigen3 CONFIG REQUIRED)
# Acts is set up by the top-level CMakeLists.txt

file(GLOB SRC_FILES CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/src/Tracking/Sim/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/Reco/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/dqm/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/geo/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/[a-zA-z]*.cxx
)

# Add individual Digitization sources.
list(APPEND SRC_FILES
  ${PROJECT_SOURCE_DIR}/src/Tracking/Digitization/ChargeCarrier.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/Digitization/PulseShape.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/Digitization/SiStripDigitizer.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/Digitization/StripClusterer.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/Digitization/StripPulseFitter.cxx
)

setup_library(module Tracking
              dependencies Framework::Configure 
                           Framework::Framework 
                           Acts::Core
                           Geant4::Interface
                           ROOT::Physics
                           Tracking::Event
                           Ecal::Event
                           DetDescr::DetDescr
              sources ${SRC_FILES})

setup_python(package_name LDMX/Tracking)

# Install data files (e.g. DAQ maps) to install/data/Tracking.
setup_data(module Tracking)
