
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(EventProc VERSION 2.1.0
                  DESCRIPTION "Module that containts event processors used by LDMX."
                  LANGUAGES CXX
)

# Find all of the source files we want to add to the SimCore library
file(GLOB EVENT_PROC_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Create the SimCore shared library
add_library(EventProc SHARED ${EVENT_PROC_SRC_FILES})

# Setup the include directories 
target_include_directories(EventProc PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Setup the libraries to link against
target_link_libraries(EventProc PUBLIC 
    LDMX::Framework LDMX::Event LDMX::Tools LDMX::DetDescr LDMX::ECal)

# Define an alias 
add_library(LDMX::EventProc ALIAS EventProc)

# Install the libraries and headers
install(TARGETS EventProc
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/EventProc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

