
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

# Setup the library
setup_library(module Tools
              dependencies Framework::Framework Event::Event Conditions::Conditions
                           ONNXRuntime::Interface
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Tools)
