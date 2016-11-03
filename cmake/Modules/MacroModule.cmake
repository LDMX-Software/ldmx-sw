###############################################################################
# Macro for defining an LDMX code module
#
# The following arguments are accepted by this macro:
# 
#   NAME - the name of the module (required)
#   INCLUDE_DIR - base include directory (optional)
#   SOURCE_DIR - base source directory (optional)
#   DEPENDENCIES - list of module dependencies such as 'Event' (optional)
#   EXTRA_SOURCES - extra source files produced by this module (optional)
#   EXTRA_LINK_LIBRARIES - extra link libraries (optional)
#   EXTRA_INCLUDE_DIRS - extra include dirs required by this module (optional)
#   EXECUTABLES - source files that should be built into executables (optional)
#
# Only 'NAME' is required as an argument to this macro.  All other arguments 
# will either be assigned reasonable defaults or will not be used.
#
# The following conventions are assumed when not overridden by macro arguments:
#
# - The C++ header files are in a directory 'include' with a subdirectory like 
#   'include/${MODULE_NAME}' and have the '.h' extension.
#
# - The C++ source files are in a directory 'src' and have the extension '.cxx'.
#
# - Test programs are in the 'test' directory and define an executable 'main' 
#   function and also have the '.cxx' extension.
#
# The names of the output executables and test programs will be derived from
# the source file names using the file's base name stripped of its extension,
# with underscores replaced by dashes.  All test programs and executables will
# be installed into the output 'bin' directory so their names should be unique 
# across all modules within the project.
#
# @author Jeremy McCormick, SLAC
###############################################################################
macro(MODULE)

  # define options for this function
  set(options)
  set(oneValueArgs NAME INCLUDE_DIR SOURCE_DIR)
  set(multiValueArgs DEPENDENCIES EXTRA_SOURCES EXTRA_LINK_LIBRARIES EXTRA_INCLUDE_DIRS EXECUTABLES)
  
  # parse command options
  cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # set library name which is always the same as the module
  set(MODULE_LIBRARY_NAME ${MODULE_NAME})
  
  # set module's include dir if not provided
  if ("${MODULE_INCLUDE_DIR}" STREQUAL "")
    set(MODULE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
  endif()
  
  # set module's source dir if not provided
  if ("${MODULE_SOURCE_DIR}" STREQUAL "")
    set(MODULE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
  endif()
  
  message(STATUS "MODULE_NAME='${MODULE_NAME}'")
  message(STATUS "MODULE_INCLUDE_DIR='${MODULE_INCLUDE_DIR}'")
  message(STATUS "MODULE_SOURCE_DIR='${MODULE_SOURCE_DIR}'")
  message(STATUS "MODULE_LIBRARY_NAME='${MODULE_LIBRARY_NAME}'")
  message(STATUS "MODULE_DEPENDENCIES='${MODULE_DEPENDENCIES}'")
  message(STATUS "MODULE_EXTRA_SOURCES='${MODULE_EXTRA_SOURCES}'")
  message(STATUS "MODULE_EXTRA_LINK_LIBRARIES='${MODULE_EXTRA_LINK_LIBRARIES}'")
  message(STATUS "MODULE_EXTRA_INCLUDE_DIRS='${MODULE_EXTRA_INCLUDE_DIRS}'")
  message(STATUS "MODULE_EXECUTABLES='${MODULE_EXECUTABLES}'")

  # define current project based on module name
  project(${MODULE_NAME} CXX)
      
  # set the local include dir var
  set(${MODULE_NAME}_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
    
  # export the include dir var to the parent scope
  set(${MODULE_NAME}_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include PARENT_SCOPE)
    
  # add the module include dir to the build
  include_directories(${${MODULE_NAME}_INCLUDE_DIR})
  
  # add include directories of module dependencies
  foreach(dependency ${MODULE_DEPENDENCIES})
    include_directories(${${dependency}_INCLUDE_DIR})
  endforeach()
  
  # add extra include directories
  include_directories(${MODULE_EXTRA_INCLUDE_DIRS})
      
  # get source and header lists for building the application
  file(GLOB sources ${MODULE_SOURCE_DIR}/*.cxx)
  file(GLOB headers ${MODULE_INCLUDE_DIR}/include/*/*.h)
  
  # add the shared library to build products
  add_library(${MODULE_NAME} SHARED ${sources} ${MODULE_EXTRA_SOURCES})
  
  # add ROOT libraries to shared library target
  target_link_libraries(${PROJECT_NAME} ${MODULE_EXTRA_LINK_LIBRARIES})
  
  # install the library
  install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

  # find test programs
  file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cxx)
  
  # setup test programs from all source files in test directory
  foreach(test_source ${test_sources})
    get_filename_component(test_program ${test_source} NAME)
    string(REPLACE ".cxx" "" test_program ${test_program})
    string(REPLACE "_" "-" test_program ${test_program})
    #message(STATUS "test_program='${test_program}'")    
    add_executable(${test_program} ${test_source})
    target_link_libraries(${test_program} ${MODULE_EXTRA_LINK_LIBRARIES} ${PROJECT_NAME} ${MODULE_DEPENDENCIES})
    install(TARGETS ${test_program} DESTINATION bin)
    message(STATUS "test_program='${test_program}'")
  endforeach()
  
  # setup module executables
  foreach(executable_source ${MODULE_EXECUTABLES})
    get_filename_component(executable ${executable_source} NAME)
    string(REPLACE ".cxx" "" executable ${executable})
    string(REPLACE "_" "-" executable ${executable})
    message(STATUS "executable='${executable}'")    
    add_executable(${executable} ${executable_source} ${sources} ${headers})
    target_link_libraries(${executable} ${MODULE_EXTRA_LINK_LIBRARIES} ${MODULE_DEPENDENCIES})
    install(TARGETS ${executable} DESTINATION bin)
  endforeach()
    
endmacro(MODULE)
