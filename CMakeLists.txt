
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Framework VERSION 2.1.0
                  DESCRIPTION "Framework used to define processing pipelines and modules."
                  LANGUAGES CXX
)

# Search for the Python3 library
find_package (Python3 COMPONENTS Interpreter Development)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Search for Boost
find_package(Boost REQUIRED COMPONENTS system log)

# Setup the library
setup_library(name Framework
              dependencies Python3::Python ROOT::Hist ROOT::TreePlayer 
                           DARK::Event Boost::log
)

# Compiling the Framework library requires features introduced by the cpp 17
# standard.  
set_target_properties(Framework PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Add the fire executable
add_executable(fire ${PROJECT_SOURCE_DIR}/src/fire.cxx)

# Link to the Framework library
target_link_libraries(fire PRIVATE Framework)

# Install the fire executable
install(TARGETS fire DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Setup the test
setup_test(dependencies DARK::Framework)
