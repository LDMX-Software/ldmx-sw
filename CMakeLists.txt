
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Framework VERSION 2.1.0
                  DESCRIPTION "Framework used to define processing pipelines and modules."
                  LANGUAGES CXX
)

# Search for the Python3 library
find_package (Python3 COMPONENTS Interpreter Development)

# Search for Boost
find_package(Boost REQUIRED COMPONENTS system log filesystem thread chrono atomic regex)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Generate the ROOT dictionary.  The following allows the use of the macro 
# used to generate the dictionary. 
include("${ROOT_DIR}/RootMacros.cmake")

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
    ${PROJECT_SOURCE_DIR}/include/Framework/Version.h.in
    ${PROJECT_SOURCE_DIR}/include/Framework/Version.h
)
install(FILES ${PROJECT_SOURCE_DIR}/include/Framework/Version.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Framework/)

# Register both the event and run headers with the event bus and ROOT dictionary
register_event_object( namespace "ldmx" class "EventHeader")
register_event_object( namespace "ldmx" class "RunHeader" type "dict_only" )

# Build the event bus from the registered event objects
build_event_bus()

# Build the ROOT dictionary from registered event objects
build_dict(template ${PROJECT_SOURCE_DIR}/include/Framework/EventLinkDef.h.in
           namespace ldmx)


# Unfortunately, the headers need to be included globably for the 
# dictionary to be generated correctly. 
include_directories(${PROJECT_SOURCE_DIR}/include)

# Copy the files needed to generate the dictionary over to the installation 
# directory.  This is needed so the dictionary can be built even when the 
# source code is deleted.
file(COPY ${PROJECT_SOURCE_DIR}/include/Framework/EventDef.h 
          ${PROJECT_SOURCE_DIR}/include/Framework/EventLinkDef.h
     DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Frameowork/
)

# Generate the ROOT dictionary
root_generate_dictionary(EventDic ${CMAKE_INSTALL_PREFIX}/include/Framework/EventDef.h
    LINKDEF ${CMAKE_INSTALL_PREFIX}/include/Framework/EventLinkDef.h)


# Install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDic_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Need to define the sources here because of the addition of EventDic
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Setup the library
setup_library(name Framework
              dependencies Python3::Python 
                           ROOT::Core ROOT::Hist ROOT::TreePlayer
                           Boost::log Boost::atomic Boost::regex
              sources EventDic.cxx ${SRC_FILES}
              python_install_path ${PYTHON_INSTALL_PREFIX}
)

# Compiling the Framework library requires features introduced by the cpp 17
# standard.  
set_target_properties(Framework PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Add the fire executable
add_executable(fire ${PROJECT_SOURCE_DIR}/src/fire.cxx)

# Link to the Framework library
target_link_libraries(fire PRIVATE Framework)

# Install the fire executable
install(TARGETS fire DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Setup the test
setup_test(dependencies DARK::Framework)

