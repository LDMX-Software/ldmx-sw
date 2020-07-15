# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DetDescr VERSION 2.1.0
                 DESCRIPTION "Detector description module."
                 LANGUAGES CXX
)

# Find all of the source files we want to add to the SimCore library
file(GLOB DETDESCR_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

setup_library(name DetDescr
              dependencies LDMX::Framework)

