

# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Tools VERSION 2.1.0
              DESCRIPTION "Module containing helper classes."
              LANGUAGES CXX
)

# Search for the ONNXRuntime binaries.  If not found, download the binaries 
# to an external folder and create the targets.
find_package(ONNXRuntime 1.2.0)

If (ONNXRuntime_FOUND)
    install(DIRECTORY "${ONNXRuntime_INCLUDE_DIRS}" DESTINATION ${CMAKE_INSTALL_PREFIX}/onnx)
    install(DIRECTORY "${ONNXRuntime_LIBRARIES}"    DESTINATION ${CMAKE_INSTALL_PREFIX}/onnx)
endif()

# Setup the library
setup_library(name Tools
              dependencies DARK::Framework DARK::Event ONNXRuntime::Interface
)
