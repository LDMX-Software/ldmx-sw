
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

find_package(Boost REQUIRED COMPONENTS system log)

# Find all of the source files we want to add to the SimCore library
file(GLOB FRAMEWORK_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Create the SimCore shared library
add_library(Framework SHARED ${FRAMEWORK_SRC_FILES})

# Set some target properties
set_target_properties(Framework PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Setup the include directories 
target_include_directories(Framework PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Setup the libraries to link against 
target_link_libraries(Framework PUBLIC 
    Python3::Python
    ROOT::Hist
    ROOT::TreePlayer
    LDMX::Event
    Boost::log
)

# Define an alias 
add_library(LDMX::Framework ALIAS Framework)

# Install the libraries and headers
install(TARGETS Framework
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/Framework
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# Add the fire executable
add_executable(fire ${PROJECT_SOURCE_DIR}/src/fire.cxx)

# Link to the Framework library
target_link_libraries(fire PRIVATE Framework)

# Install the fire executable
install(TARGETS fire DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
