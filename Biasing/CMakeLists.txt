
#Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

#Set the project name
project(Biasing DESCRIPTION "Biasing classes." LANGUAGES CXX)

setup_library(
  module Biasing 
  name Utility 
  dependencies Geant4::Interface 
               Framework::Framework 
               SimCore::SimCore
)

setup_library(
  module Biasing 
  dependencies Geant4::Interface
               Framework::Framework 
               SimCore::SimCore
)

setup_python(package_name LDMX/Biasing)

# run all *.py files in test during testing
setup_test(config_dir test)
