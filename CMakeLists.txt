
# Set the minimum version of CMake that's required 
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(LDMX_SW VERSION 2.1.0
                DESCRIPTION "The Light Dark Matter eXperiment simulation and reconstruction framework." 
                LANGUAGES CXX    
)

# If an install location hasn't been set via CMAKE_INSTALL_PREFIX, set it to 
# a reasonable default ($PWD/install).  
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install CACHE PATH "" FORCE)
    message(STATUS "Install directory set to ${CMAKE_INSTALL_PREFIX}")
endif()

if(NOT EXISTS /.dockerenv AND NOT EXISTS /singularity)
  message(WARNING "You are not inside a container; you may be working in an untested environment.")
endif()

# Set the default release type to "Release". If a release type is specified 
# at the command line, it's respected.
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()

# These two modules are the minimum needed to build and run the framework.
add_subdirectory(Event)
add_subdirectory(Framework)

# To build SimCore, the DetDescr package is required
add_subdirectory(DetDescr)

# SimCore allows the simulation to be run without biasing
add_subdirectory(SimCore)

# Adding the Biasing module, allows the use of biasing in the simulation
add_subdirectory(Biasing)

# Add the Tools module.
add_subdirectory(Tools)

add_subdirectory(Ecal)

add_subdirectory(EventProc)

# Add the Detectors submodule.  This has no dependecies.
add_subdirectory(Detectors)

# 
add_subdirectory(DQM)

option(BUILD_EVE "Build the event display application. This requires more components of ROOT." OFF)
if(BUILD_EVE)
    add_subdirectory(EventDisplay)
endif()
# add dir with extra CMake modules 
#list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# find ONNXRuntime installation
# specify location with -DONNXRUNTIME_ROOT=<location-of-onnxruntime-prefix>
#find_package(ONNXRuntime)
#if(ONNXRUNTIME_FOUND)
#  message(STATUS "ONNXRuntime libs found at: ${ONNXRUNTIME_LIBRARIES}")
#  message(STATUS "ONNXRuntime include dir found at: ${ONNXRUNTIME_INCLUDE_DIRS}")
#  add_definitions(-DLDMX_USE_ONNXRUNTIME)
#else()
#  message(WARNING "Cannot find ONNXRuntime: will not be able to run EcalVeto BDT or DNN")
#endif()

# Magnetic field map installation
option(INSTALL_FIELDMAP "Install field map data from MagFieldMap sub-module" ON)
message(STATUS "MagFieldMap installation: ${INSTALL_FIELDMAP}")
if(INSTALL_FIELDMAP)
  install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Scripts/InstallMagFieldMap.cmake")
endif()

# build and install test executable ldmx-test
#option(BUILD_TESTS "Build and link ldmx-test executable." OFF)
#if(BUILD_TESTS)
#  include(TestExecutable)
#endif()

# install python init file for top-level LDMX module
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py 
    "\"\"\"Python package to configure ldmx-sw\"\"\"")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION lib/python/LDMX)

