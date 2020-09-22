# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DetDescr VERSION 2.1.0
                 DESCRIPTION "Detector description module."
                 LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

setup_library(name DetDescr
              dependencies  ROOT::Core ROOT::Hist
                            Framework::Exception Framework::Configure 
)

# Setup the test
setup_test(dependencies DARK::DetDescr)

# Add the printing executable
add_executable(print-ecal-hex-readout ${PROJECT_SOURCE_DIR}/src/DetDescr/print_ecal_hex_readout.cxx)

# Link to the Framework library
target_link_libraries(print-ecal-hex-readout PRIVATE DetDescr ROOT::Gpad)

# Install the fire executable
install(TARGETS print-ecal-hex-readout DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

setup_python(install_path ${PYTHON_INSTALL_PREFIX}/DetDescr)
