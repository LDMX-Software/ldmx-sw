# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  Packing
  VERSION 1.0.0
  DESCRIPTION "Classes needed to interface with raw binary data."
  LANGUAGES CXX)

setup_library(module Packing name Utility interface)

# RawDataFile submodule
setup_library(module Packing name RawDataFile dependencies Packing::Utility)

# Setup the library
setup_library(module Packing
              dependencies Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr
                           Packing::RawDataFile)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Packing)

setup_test(dependencies Packing::Packing)
