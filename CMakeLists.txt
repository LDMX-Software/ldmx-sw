
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name and version.
# TODO: The version should be synced up with ldmx-sw.
project(Tracking VERSION 2.1.0
                 DESCRIPTION "Module containing the sim and recon code for the trackers."
                 LANGUAGES CXX
)

# Allow building of event specific classes only.  The most common use case is 
# needing to build the event classes to allow building of the event bus and ROOT
# dictionary prior building the rest of the modules in a package. 
option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

    # Search for ROOT and add all targets.
    find_package(ROOT CONFIG REQUIRED)


    # Register all of the classes with the event bus.
    register_event_object(module_path "Tracking/Event" 
	                  namespace "ldmx" 
	                  class "RawSiStripHit" )

    # Get all of the event sources that will be included in this library
    file(GLOB EVENT_SRC_FILES CONFIGURE_DEPENDS 
	      ${PROJECT_SOURCE_DIR}/src/Tracking/Event/[a-zA-z]*.cxx)

    # Generate the files needed to build the event classes.
    setup_library(module Tracking
                  name Event
                  dependencies ROOT::Core
                  sources ${EVENT_SRC_FILES}
                  register_target)

     return()
endif()

file(GLOB SRC_FILES CONFIGURE_DEPENDS 
	  ${PROJECT_SOURCE_DIR}/src/Tracking/Sim/[a-zA-z]*.cxx
)

setup_library(module Tracking
              dependencies Framework::Configure 
                           Framework::Framework 
              sources ${SRC_FILES})

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Tracking)
