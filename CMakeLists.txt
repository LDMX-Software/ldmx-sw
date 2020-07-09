
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(SimCore VERSION 2.1.0
                DESCRIPTION "Framework used to define processing pipelines and modules."
                LANGUAGES CXX
)

# Search for the Python3 library
find_package (Python3 COMPONENTS Interpreter Development)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

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
)

# declare Event module
#module(
#  NAME Framework  
#  EXECUTABLES src/fire.cxx
#  DEPENDENCIES Event 
#  EXTERNAL_DEPENDENCIES ROOT Python
#)
