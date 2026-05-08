cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  Packing
  DESCRIPTION "Classes needed to interface with raw binary data."
  LANGUAGES CXX)

setup_library(module Packing name Utility)

# RawDataFile submodule
setup_library(module Packing name RawDataFile 
              dependencies Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr
                           Packing::Utility)

# Setup the library
setup_library(module Packing
              dependencies Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr
                           Packing::RawDataFile)

setup_python(package_name LDMX/Packing)

setup_test(dependencies Packing::Packing)

add_executable(rogue-extract app/rogue_extract.cxx)
target_link_libraries(rogue-extract PRIVATE Packing::Packing)
install(TARGETS rogue-extract)
