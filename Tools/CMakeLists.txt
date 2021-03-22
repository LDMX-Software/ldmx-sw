
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

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

# Setup the library
setup_library(module Tools
              dependencies Framework::Framework Recon::Event Conditions::Conditions
                           ONNXRuntime::Interface ROOT::Core ROOT::Physics
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Tools)

# Add the hgcroc running executable
add_executable(run-hgcroc ${PROJECT_SOURCE_DIR}/src/Tools/run_hgcroc.cxx)
target_link_libraries(run-hgcroc PRIVATE Framework Tools)
install(TARGETS run-hgcroc DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
