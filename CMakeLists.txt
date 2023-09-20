# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  Framework
  VERSION 1.3.6
  DESCRIPTION "Framework used to define processing pipelines and modules."
  LANGUAGES CXX)

setup_library(module Framework name Exception sources
              ${PROJECT_SOURCE_DIR}/src/Framework/Exception/Exception.cxx)

setup_library(module Framework name Configure interface)

# Search for the Python3 library
find_package(Python3 COMPONENTS Interpreter Development)

# Search for Boost
find_package(
  Boost REQUIRED
  COMPONENTS system
             log
             filesystem
             thread
             chrono
             atomic
             regex)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Generate the ROOT dictionary.  The following allows the use of the macro used
# to generate the dictionary.
include("${ROOT_DIR}/RootMacros.cmake")

# Execute the command to extract the SHA1 hash of the current git tag. The
# variable GIT_SHA1 will be set to contain the hash.
execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE GIT_SHA1)

# Remove the newline character
string(REGEX REPLACE "\n$" "" GIT_SHA1 "${GIT_SHA1}")

# Copies the file 'Version.h.in', substitutes the value of GIT_SHA1 and writes
# it out to Version.h.
configure_file(${PROJECT_SOURCE_DIR}/include/Framework/Version.h.in
               ${PROJECT_BINARY_DIR}/include/Framework/Version.h)
install(FILES ${PROJECT_BINARY_DIR}/include/Framework/Version.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Framework/)

# Register both the event and run headers with the event bus and ROOT dictionary
register_event_object(module_path "Framework" namespace "ldmx" 
                      class "EventHeader")
register_event_object(module_path "Framework" namespace "ldmx" 
                      class "RunHeader")

# Setup the library
setup_library(module Framework 
  dependencies Python3::Python
               ROOT::Core
               ROOT::Hist
               ROOT::TreePlayer
               Boost::log
               Boost::atomic
               Boost::regex
               Framework::Exception
               Framework::Configure
               "${registered_targets}")

# Compiling the Framework library requires features introduced by the cpp 17
# standard.
set_target_properties(
  Framework
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED YES
             CXX_EXTENSIONS NO)

# the following writes the LinkDef file using the CMake global variables
# namespaces and dict that are lists appended to by the register_event_object function
message(STATUS "Building ROOT dictionary LinkDef")
set(linkdef_filepath ${PROJECT_BINARY_DIR}/include/Framework/EventDictLinkDef.h)
file(APPEND ${linkdef_filepath} "
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

")
list(REMOVE_DUPLICATES namespaces)
foreach(namespace ${namespaces})
  file(APPEND ${linkdef_filepath} "#pragma link C++ namespace ${namespace};\n")
endforeach()

foreach(entry ${dict})
  file(APPEND ${linkdef_filepath} "#pragma link C++ class ${entry}+;\n")
endforeach()

file(APPEND ${linkdef_filepath} "\n#endif\n")

# need to update include directories so the dictionary generation knows
# to use different paths at install time
target_include_directories(Framework PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  ${include_paths}
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

# attach a ROOT dictionary to the Framework module with all the registered
# headers in the event_headers list
root_generate_dictionary(EventDict
  ${event_headers}
  LINKDEF ${linkdef_filepath}
  MODULE Framework)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libFramework_rdict.pcm DESTINATION lib)

# Add the fire executable
add_executable(fire ${PROJECT_SOURCE_DIR}/app/fire.cxx)
target_link_libraries(fire PRIVATE Framework::Framework)
install(TARGETS fire DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Setup the test
setup_test(dependencies Framework::Framework)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Framework)
