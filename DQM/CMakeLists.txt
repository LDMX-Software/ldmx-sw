
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DQM VERSION 2.1.0
            DESCRIPTION "Data quality management tools."
            LANGUAGES CXX
)

# Find all of the source files we want to add to the SimCore library
file(GLOB DQM_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Create the SimCore shared library
add_library(DQM SHARED ${DQM_SRC_FILES})

# Setup the include directories 
target_include_directories(DQM PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Setup the libraries to link against
target_link_libraries(DQM PUBLIC 
    LDMX::Framework LDMX::Event LDMX::Tools LDMX::DetDescr)

# Define an alias 
add_library(LDMX::DQM ALIAS DQM)

# Install the libraries and headers
install(TARGETS DQM
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/DQM
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
