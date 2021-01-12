# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Trigger VERSION 2.1.0
                  DESCRIPTION "Module for trigger algorithms."
                  LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

include_directories(../submodules/HLS_arbitrary_Precision_Types/include)

setup_library(module Trigger
              dependencies Framework::Framework Recon::Event DetDescr::DetDescr
                           Tools::Tools SimCore::Event Ecal::Ecal
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Trigger)
