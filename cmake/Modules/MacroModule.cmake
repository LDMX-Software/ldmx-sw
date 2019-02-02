###############################################################################
# Macro for defining an LDMX code module
#
# The following arguments are accepted by this macro:
# 
#   NAME - the name of the module (required)
#   DEPENDENCIES - list of module dependencies such as 'Event' (optional)
#   EXTRA_SOURCES - extra source files produced by this module (optional)
#   EXTRA_LINK_LIBRARIES - extra link libraries (optional)
#   EXTRA_INCLUDE_DIRS - extra include dirs required by this module (optional)
#   EXECUTABLES - source files that should be built into executables (optional)
#
# Only 'NAME' is required as an argument to this macro.  All other arguments 
# will either be assigned reasonable defaults or will not be used.
#
# The following conventions are assumed:
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
# across all modules within the repository.
#
# @author Jeremy McCormick, SLAC
###############################################################################
macro(MODULE)

  # define options for this function
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs DEPENDENCIES EXTRA_SOURCES EXTRA_LINK_LIBRARIES EXECUTABLES EXTERNAL_DEPENDENCIES)
  
  # parse command options
  cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # set module's include dir
  set(MODULE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
  
  # set module's source dir
  set(MODULE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

  # print debug info 
  if(MODULE_DEBUG) 
    message("MODULE_NAME='${MODULE_NAME}'")
    message("MODULE_DEPENDENCIES='${MODULE_DEPENDENCIES}'")
    message("MODULE_EXTERNAL_DEPENDENCIES='${MODULE_EXTERNAL_DEPENDENCIES}'")
    message("MODULE_INCLUDE_DIR='${MODULE_INCLUDE_DIR}'")
    message("MODULE_SOURCE_DIR='${MODULE_SOURCE_DIR}'")
    message("MODULE_EXTRA_SOURCES='${MODULE_EXTRA_SOURCES}'")
    message("MODULE_EXTRA_LINK_LIBRARIES='${MODULE_EXTRA_LINK_LIBRARIES}'")
    message("MODULE_EXECUTABLES='${MODULE_EXECUTABLES}'")
  endif()

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
  
  # get source and header lists for building the application
  file(GLOB sources ${MODULE_SOURCE_DIR}/*.cxx)
  file(GLOB headers ${MODULE_INCLUDE_DIR}/include/*/*.h)

  # setup external dependencies
  ext_deps(DEPENDENCIES ${MODULE_EXTERNAL_DEPENDENCIES}) 
  if (EXT_DEP_INCLUDE_DIRS)
    include_directories(${EXT_DEP_INCLUDE_DIRS})
  endif()

  # make list of all library dependencies
  set(MODULE_LIBRARIES ${MODULE_DEPENDENCIES} ${EXT_DEP_LIBRARIES} ${MODULE_EXTRA_LINK_LIBRARIES})
  if(MODULE_DEBUG)
    message("MODULE_LIBRARIES='${MODULE_LIBRARIES}'")
  endif()

  # if there are C++ source files then build a shared library
  if (sources)

    # add library target
    add_library(${MODULE_NAME} SHARED ${sources} ${MODULE_EXTRA_SOURCES})
   
    # add link libs
    target_link_libraries(${MODULE_NAME} ${MODULE_EXTRA_LINK_LIBRARIES})
  
    # install the library
    install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

  endif()
  
  # make list of libraries required by executables and test programs which includes this module's lib
  if (sources)
    set(MODULE_BIN_LIBRARIES ${MODULE_NAME} ${MODULE_LIBRARIES})
  else()
    set(MODULE_BIN_LIBRARIES ${MODULE_LIBRARIES})
  endif()
    
  if(MODULE_DEBUG)
    message("MODULE_BIN_LIBRARIES='${MODULE_BIN_LIBRARIES}'")
  endif()
 
  # find test programs
  file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cxx)

  # setup test programs from all source files in test directory
  foreach(test_source ${test_sources})
    get_filename_component(test_program ${test_source} NAME)
    string(REPLACE ".cxx" "" test_program ${test_program})
    string(REPLACE "_" "-" test_program ${test_program})
    add_executable(${test_program} ${test_source})
    target_link_libraries(${test_program} ${MODULE_BIN_LIBRARIES})
    install(TARGETS ${test_program} DESTINATION bin)
    if(MODULE_DEBUG)
      message("building test program: ${test_program}")
    endif()
  endforeach()
  
  # setup module executables
  foreach(executable_source ${MODULE_EXECUTABLES})
    get_filename_component(executable ${executable_source} NAME)
    string(REPLACE ".cxx" "" executable ${executable})
    string(REPLACE "_" "-" executable ${executable})
    if(MODULE_DEBUG)
      message("building executable: ${executable}")
    endif()
    add_executable(${executable} ${executable_source})
    target_link_libraries(${executable} ${MODULE_BIN_LIBRARIES})
    install(TARGETS ${executable} DESTINATION bin)
  endforeach()

  # install python scripts
  set(PYTHON_INSTALL_DIR lib/python/LDMX/${MODULE_NAME})
  file(GLOB py_scripts "${CMAKE_CURRENT_SOURCE_DIR}/python/[!_]*.py")
  if (py_scripts)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py "# python package")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION ${PYTHON_INSTALL_DIR})
  endif()
  
  # install python programs 
  foreach(pyscript ${py_scripts})
    install(FILES ${pyscript} DESTINATION ${PYTHON_INSTALL_DIR})
    if(MODULE_DEBUG)
      message("installing python script: ${pyscript}")
    endif()
  endforeach()

  if (MODULE_DEBUG)
    message("")
  endif()
  
endmacro(MODULE)
