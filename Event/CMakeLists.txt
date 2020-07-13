
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Event VERSION 2.1.0
              DESCRIPTION "Event classes used to persist LDMX simulation and physics objects." 
              LANGUAGES CXX
)

# Execute the command to extract the SHA1 hash of the current git tag. 
# The variable GIT_SHA1 will be set to contain the hash. 
execute_process (
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_SHA1
)

# Remove the newline character
string(REGEX REPLACE "\n$" "" GIT_SHA1 "${GIT_SHA1}")

# Copies the file 'Version.h.in', substitutes the value of GIT_SHA1 and
# writes it out to Version.h.  
configure_file(
    ${PROJECT_SOURCE_DIR}/include/Event/Version.h.in
    ${PROJECT_SOURCE_DIR}/include/Event/Version.h
)
install(FILES ${PROJECT_SOURCE_DIR}/include/Event/Version.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Event/)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Generate the ROOT dictionary.  The following allows the use of the macro 
# used to generate the dictionary. 
include("${ROOT_DIR}/RootMacros.cmake")

# Unfortunately, the Event headers need to be included globably for the 
# dictionary to be generated correctly. 
include_directories(${PROJECT_SOURCE_DIR}/include)

# Generate the ROOT dictionary
root_generate_dictionary(EventDic ${PROJECT_SOURCE_DIR}/include/Event/EventDef.h 
    LINKDEF ${PROJECT_SOURCE_DIR}/include/Event/EventLinkDef.h)

# Install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDic_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Need to define the sources here because of the addition of EventDic
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Setup the library
setup_library(name Event
              dependencies ROOT::Core
              sources EventDic.cxx ${SRC_FILES}
)

# If this is really needed, import a target instead.
# Add the location of Version.h to the build. Also, set the variable
# EXTRA_INC in the parent scope so the extra include directory can 
# be added to the whole project not just this module.
#include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
#set(EXTRA_INC ${CMAKE_CURRENT_BINARY_DIR}/include/ PARENT_SCOPE)$
