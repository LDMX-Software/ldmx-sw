
#Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

#Set the project name
project(Biasing VERSION 2.1.0 DESCRIPTION "Biasing classes." LANGUAGES CXX)

#Configure Geant4
setup_geant4_target()

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

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Biasing)
