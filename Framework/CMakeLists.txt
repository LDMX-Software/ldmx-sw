cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  Framework
  DESCRIPTION "Framework used to define processing pipelines and modules."
  LANGUAGES CXX)

setup_library(module Framework name Exception)

setup_library(module Framework name Performance dependencies ROOT::Core Framework::Exception)
root_generate_dictionary(PerfDict
  Framework/Performance/Timer.h
  LINKDEF ${PROJECT_SOURCE_DIR}/include/Framework/Performance/LinkDef.h
  MODULE Framework_Performance)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libFramework_Performance_rdict.pcm DESTINATION lib)

setup_library(module Framework name Configure dependencies Python3::Python)

set(GIT_SHA1 "${COMMIT_SHA}")
message(STATUS "Deduced git SHA: ${GIT_SHA1}")


# Copies the file 'Version.h.in', substitutes the value of GIT_SHA1 and writes
# it out to Version.h.
configure_file(${PROJECT_SOURCE_DIR}/include/Framework/Version.h.in
               ${PROJECT_BINARY_DIR}/include/Framework/Version.h)
install(FILES ${PROJECT_BINARY_DIR}/include/Framework/Version.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Framework/)

# Setup the library
setup_library(module Framework 
  dependencies ROOT::Core
               ROOT::Hist
               ROOT::TreePlayer
               Boost::log
               Boost::atomic
               Boost::regex
               Framework::Exception
               Framework::Configure
               Framework::Performance
  linkdef include/Framework/LinkDef.h)
target_include_directories(Framework PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

# Compiling the Framework library requires features introduced by the cpp 17
# standard.
set_target_properties(
  Framework
  PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD}
             CXX_STANDARD_REQUIRED YES
             CXX_EXTENSIONS NO)

# Add the fire executable
add_executable(fire ${PROJECT_SOURCE_DIR}/app/fire.cxx)
target_link_libraries(fire PRIVATE Framework::Framework)
install(TARGETS fire DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES app/fire-parallel
  TYPE BIN
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
              GROUP_READ GROUP_EXECUTE
              WORLD_READ WORLD_EXECUTE)

# Setup the test
setup_test(dependencies Framework::Framework)

setup_python(package_name LDMX/Framework)
