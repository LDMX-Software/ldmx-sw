
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Biasing VERSION 2.1.0
                DESCRIPTION "Biasing classes."
                LANGUAGES CXX
)

# Configure Geant4
find_package(Geant4 REQUIRED gdml ui_all vis_all)

# Export the Geant4 target if it hasn't been done yet.
if( NOT TARGET Geant4::Interface) 
    
    # Create the Geant4 target
    add_library(Geant4::Interface INTERFACE IMPORTED GLOBAL)

    # Set the target properties
    SET_TARGET_PROPERTIES(Geant4::Interface
        PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${Geant4_Flags}"
        INTERFACE_COMPILE_DEFINITIONS "${Geant4_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Geant4_INCLUDE_DIRS}"
    )

    message(STATUS "Found Geant4 version ${Geant4_VERSION}")

endif()

# Find all of the source files we want to add to the SimCore library
file(GLOB BIASING_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Create the SimCore shared library
add_library(Biasing SHARED ${BIASING_SRC_FILES})

# Setup the include directories 
target_include_directories(Biasing PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Setup the libraries to link against
target_link_libraries(Biasing PUBLIC Geant4::Interface LDMX::Framework LDMX::SimCore)

# Define an alias
add_library(LDMX::Biasing ALIAS Biasing)

# Install the libraries and headers
install(TARGETS Biasing
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/Biasing
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
