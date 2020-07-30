
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Ecal VERSION 2.1.0
             DESCRIPTION "Module for all things ECal."
             LANGUAGES CXX
)


# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

# Search for the ONNXRuntime binaries.  If not found, download the binaries 
# to an external folder and create the targets.
find_package(ONNXRuntime 1.2.0)

setup_library(name Ecal
              dependencies ROOT::Physics DARK::Framework DARK::Event DARK::Tools 
              DARK::DetDescr ONNXRuntime::Interface
              python_install_path ${CMAKE_INSTALL_PREFIX}/python/LDMX
)

setup_test(dependencies DARK::Ecal)
