
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(SimCore VERSION 2.1.0
                DESCRIPTION "Core classes needed to run the Simulator."
                LANGUAGES CXX
)

include(CMakePrintHelpers)

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
file(GLOB SIMCORE_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Create the SimCore shared library
add_library(SimCore SHARED ${SIMCORE_SRC_FILES})

# Set some target properties
set_target_properties(SimCore PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Setup the include directories 
target_include_directories(SimCore PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Setup the libraries to link against
target_link_libraries(SimCore PUBLIC Geant4::Interface)

