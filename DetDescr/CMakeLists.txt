# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DetDescr VERSION 2.1.0
                 DESCRIPTION "Detector description module."
                 LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Find all of the source files we want to add to the SimCore library
file(GLOB DETDESCR_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Create the SimCore shared library
add_library(DetDescr SHARED ${DETDESCR_SRC_FILES})

# Setup the include directories 
target_include_directories(DetDescr PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Setup the libraries to link against
target_link_libraries(DetDescr PUBLIC LDMX::Framework)

# Define an alias 
add_library(LDMX::DetDescr ALIAS DetDescr)

# Install the libraries and headers
install(TARGETS DetDescr
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/DetDescr
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
