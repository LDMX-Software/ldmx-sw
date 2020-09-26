# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Detector VERSION 2.1.0
                 DESCRIPTION "Detector description module."
                 LANGUAGES CXX
)

if(NOT TARGET DD4hep::DDCore)
    find_package(DD4hep REQUIRED)
    include( ${DD4hep_DIR}/cmake/DD4hep.cmake)
    include( ${DD4hep_DIR}/cmake/DD4hepBuild.cmake)
    dd4hep_configure_output()
endif()

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

if(TARGET XercesC::XercesC)
  SET(OPT_XERCESC XercesC::XercesC)
endif()

dd4hep_add_plugin(Tracker SOURCES ${PROJECT_SOURCE_DIR}/src/Detectors/*.cxx
    USES DD4hep::DDCore ROOT::Core ROOT::Geom ROOT::GenVector ${OPT_XERCESC})

install(TARGETS Tracker LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX}/data/compact)


