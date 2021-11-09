# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Detector VERSION 2.1.0
                 DESCRIPTION "Detector description module."
                 LANGUAGES CXX
)


# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

setup_dd4hep_target()

# Find the eigen library
find_package(Eigen3 REQUIRED)

file(GLOB SRC_FILES CONFIGURE_DEPENDS
     ${PROJECT_SOURCE_DIR}/src/Detectors/Tracking/[a-zA-z]*.cxx
     ${PROJECT_SOURCE_DIR}/src/Detectors/Geo/[a-zA-z]*.cxx
)

setup_library(module Detectors
              dependencies DD4hep::DDCore
              sources ${SRC_FILES})

dd4hep_add_plugin(Tracker SOURCES ${PROJECT_SOURCE_DIR}/src/Detectors/DD4hep/*.cxx
                  USES DD4hep::DDCore ROOT::Core ROOT::Geom ROOT::GenVector 
                  Detectors::Detectors DetDescr::DetDescr 
                  Framework::Framework ${OPT_XERCESC})
  
install(TARGETS Tracker LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX}/data/compact)

