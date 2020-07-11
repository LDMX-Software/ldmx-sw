
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(ECal VERSION 2.1.0
             DESCRIPTION "Module for all things ECal."
             LANGUAGES CXX
)

# Find all of the source files we want to add to the SimCore library
file(GLOB ECAL_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Create the SimCore shared library
add_library(ECal SHARED ${ECAL_SRC_FILES})

# Setup the include directories 
target_include_directories(ECal PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Setup the libraries to link against
target_link_libraries(ECal PUBLIC LDMX::Framework LDMX::Event LDMX::Tools LDMX::DetDescr)

# Define an alias 
add_library(LDMX::ECal ALIAS ECal)

# Install the libraries and headers
install(TARGETS ECal
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/Ecal
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

