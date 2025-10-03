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
# if you want to try an alternate Acts, you can
# 1. comment out the stuff here and replace with add_subdirectory(acts)
# 2. git clone acts into this directory and choose your version
find_package(Acts QUIET)
if (NOT Acts_FOUND)
  message(STATUS "Did not find Acts, downloading and compiling it here")
  include(FetchContent)
  FetchContent_Declare(
      Acts
      URL https://github.com/acts-project/acts/archive/refs/tags/v36.0.0.tar.gz
      URL_HASH MD5=f543dd8ba030bea2e4ee2f1b07dbe7c0
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(Acts)
  FetchContent_GetProperties(Acts)
  # Adding Acts as "SYSTEM" 
  # which will silence compiler warnings from these 3rd party softwares
  include_directories(SYSTEM "${Acts_SOURCE_DIR}/Core/include")
else()
  message(STATUS "Found Acts ${Acts_VERSION}")
endif()

file(GLOB SRC_FILES CONFIGURE_DEPENDS 
  ${PROJECT_SOURCE_DIR}/src/Tracking/Sim/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/Reco/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/dqm/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/geo/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Tracking/[a-zA-z]*.cxx
)

setup_library(module Tracking
              dependencies Framework::Configure 
                           Framework::Framework 
                           ActsCore
                           Geant4::Interface
                           ROOT::Physics
                           Tracking::Event
                           Ecal::Event
              sources ${SRC_FILES})

setup_python(package_name LDMX/Tracking)
