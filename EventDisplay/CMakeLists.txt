
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(EventDisplay VERSION 2.1.0
                     DESCRIPTION "Event display module." 
                     LANGUAGES CXX
)

# Search for the Python3 library
find_package (Python3 COMPONENTS Interpreter Development)

# Search and configure ROOT
find_package(ROOT 6.20 CONFIG REQUIRED)

# Clear any variables cached during previous configuration cycles. 
clear_cache_variables()

# Generate the ROOT dictionary.  The following allows the use of the macro 
# used to generate the dictionary. 
include("${ROOT_DIR}/RootMacros.cmake")

# Register the EventDisplay class with the ROOT dictionary
register_event_object(module_path "EventDisplay" namespace "eventdisplay"
                      class "Display")

build_event_bus(path ${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayDef.h)

build_dict(name "EventDisplay" namespace ldmx 
  template ${LDMX_SW_SOURCE_DIR}/Framework/include/Framework/EventLinkDef.h.in)

# Unfortunately, the Event headers need to be included globably for the 
# dictionary to be generated correctly.
#include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LDMX_SW_SOURCE_DIR}/DetDescr/include)
include_directories(${include_paths})

# Copy the files needed to generate the dictionary over to the installation
# directory.  This is needed so the dictionary can be built even when the source
# code is deleted.
file(COPY ${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayDef.h
          ${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayLinkDef.h
          DESTINATION ${CMAKE_INSTALL_PREFIX}/include/EventDisplay/)

# Generate the ROOT dictionary
root_generate_dictionary(EventDisplayDic ${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayDef.h 
    LINKDEF ${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayLinkDef.h)

# When issues the make clean command, remove both the EventDef and EventLinkDef
# files.
set_directory_properties(
  PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES
    "${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayDef.h;${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayLinkDef.h"
)

# Install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDisplayDic_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Find all of the source files we want to add to the Event library
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/EventDisplay/[a-zA-Z]*.cxx)

setup_library(module EventDisplay
  dependencies Python3::Python
    ROOT::Core
    ROOT::Rint
    ROOT::Geom
    ROOT::Gui
    ROOT::Eve
    DetDescr::DetDescr
    Framework::Framework
    SimCore::Event
    Hcal::Event
    Ecal::Event
    sources EventDisplayDic.cxx
    ${SRC_FILES}
)
  
# Add the event display executable
add_executable(eve ${PROJECT_SOURCE_DIR}/src/EventDisplay/ldmx_eve.cxx)
target_link_libraries(eve PRIVATE EventDisplay::EventDisplay)
install(TARGETS eve DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

