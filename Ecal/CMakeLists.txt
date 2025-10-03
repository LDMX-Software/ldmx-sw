cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Ecal DESCRIPTION "Module for all things ECal."
             LANGUAGES CXX
)

setup_library(
  module Ecal
  name Event
  dependencies ROOT::Core ROOT::Physics Recon::Event
  linkdef ${CMAKE_CURRENT_SOURCE_DIR}/include/Ecal/Event/LinkDef.h
)

# Search for the ONNXRuntime binaries.  If not found, download the binaries 
# to an external folder and create the targets.
find_package(ONNXRuntime 1.2.0)

find_package(Eigen3 CONFIG REQUIRED)

setup_library(module Ecal
              dependencies ROOT::Physics
                           Framework::Framework Recon::Event Tools::Tools DetDescr::DetDescr 
                           ONNXRuntime::Interface Ecal::Event Tracking::Event SimCore::Event
                           Eigen3::Eigen
)

setup_test(dependencies Ecal::Ecal)

setup_python(package_name LDMX/Ecal)

setup_data(module Ecal)

# Add the printing executable
add_executable(print-ecal-hex-readout ${PROJECT_SOURCE_DIR}/src/Ecal/print_ecal_hex_readout.cxx)

# Link to the Framework library
target_link_libraries(print-ecal-hex-readout PRIVATE DetDescr Ecal ROOT::Physics)

# Install the fire executable
install(TARGETS print-ecal-hex-readout DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
