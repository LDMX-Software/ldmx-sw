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
# - Test programs are in the 'test' directory and define an catch2 TEST_CASE
#   function and also have the '.cxx' extension.
#
# The names of the output executables and test programs will be derived from
# the source file names using the file's base name stripped of its extension,
# with underscores replaced by dashes.  
#
# All test programs are compiled against Exception/test/ldmx_test.cxx
#   @sa cmake/Modules/TestExecutable.cmake
#
# @author Jeremy McCormick, SLAC
# @author Tom Eichlersmith, University of Minnesota
###############################################################################

macro(MODULE)

  # define options for this function
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs DEPENDENCIES EXTRA_SOURCES EXTRA_LINK_LIBRARIES EXECUTABLES EXTERNAL_DEPENDENCIES)
  
  # parse command options
  cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # define current project based on module name
  project(${MODULE_NAME} CXX)

  # set the module's directory information
  set(${MODULE_NAME}_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)  #include 
  set(${MODULE_NAME}_SOURCE_DIR  ${PROJECT_SOURCE_DIR}/src)      #src
  set(${MODULE_NAME}_PYTHON_DIR  ${PROJECT_SOURCE_DIR}/python)   #python 
  set(${MODULE_NAME}_DATA_DIR    ${PROJECT_SOURCE_DIR}/data)     #data
  set(${MODULE_NAME}_TEST_DIR    ${PROJECT_SOURCE_DIR}/test)     #data

  # export the directory variables to the parent scope (outside this module function)
  #   used by other modules to include
  set(${MODULE_NAME}_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include PARENT_SCOPE)
  set(${MODULE_NAME}_SOURCE_DIR  ${PROJECT_SOURCE_DIR}/src     PARENT_SCOPE)
  set(${MODULE_NAME}_PYTHON_DIR  ${PROJECT_SOURCE_DIR}/python  PARENT_SCOPE)
  set(${MODULE_NAME}_DATA_DIR    ${PROJECT_SOURCE_DIR}/data    PARENT_SCOPE)
  set(${MODULE_NAME}_TEST_DIR    ${PROJECT_SOURCE_DIR}/test    PARENT_SCOPE)

  # include the headers (and install them if they exist)
  include_directories(${${MODULE_NAME}_INCLUDE_DIR})
  if(EXISTS ${${MODULE_NAME}_INCLUDE_DIR})
    # module include directory exists ==> install
    install(DIRECTORY ${${MODULE_NAME}_INCLUDE_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX})
  endif()
    
  # add include directories of module dependencies
  foreach(dependency ${MODULE_DEPENDENCIES})
    if ( EXISTS "${${dependency}_INCLUDE_DIR}" )
      include_directories(${${dependency}_INCLUDE_DIR})
    endif()
  endforeach()
  
  # setup external dependencies
  ext_deps(DEPENDENCIES ${MODULE_EXTERNAL_DEPENDENCIES}) 
  if (EXT_DEP_INCLUDE_DIRS)
    include_directories(${EXT_DEP_INCLUDE_DIRS})
  endif()

  # make list of all library dependencies
  set(MODULE_LIBRARIES ${MODULE_DEPENDENCIES} ${EXT_DEP_LIBRARIES} ${MODULE_EXTRA_LINK_LIBRARIES})

  # get source and header lists for building the application
  file(GLOB sources ${${MODULE_NAME}_SOURCE_DIR}/*.cxx)

  # if there are C++ source files then build a shared library
  if (sources)

    # add library target
    add_library(${MODULE_NAME} SHARED ${sources} ${MODULE_EXTRA_SOURCES})
   
    # add link libs
    target_link_libraries(${MODULE_NAME} ${MODULE_LIBRARIES})
  
    # install the library
    install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

    # make list of libraries required by executables and test programs which includes this module's lib
    set(${MODULE_NAME}_BIN_LIBRARIES ${MODULE_NAME} ${MODULE_LIBRARIES})
  else()
    # no sources ==> still list module libraries for test executables
    set(${MODULE_NAME}_BIN_LIBRARIES ${MODULE_LIBRARIES})
  endif()

  # setup module executables
  foreach(executable_source ${MODULE_EXECUTABLES})
    get_filename_component(executable ${executable_source} NAME)
    string(REPLACE ".cxx" "" executable ${executable})
    string(REPLACE "_" "-" executable ${executable})
    if(MODULE_DEBUG)
      message("building executable: ${executable}")
    endif()
    add_executable(${executable} ${executable_source})
    target_link_libraries(${executable} ${${MODULE_NAME}_BIN_LIBRARIES})
    install(TARGETS ${executable} DESTINATION bin)
  endforeach()

  set(PYTHON_INSTALL_DIR lib/python/LDMX/${MODULE_NAME})

  # install python scripts
  file(GLOB py_scripts "${${MODULE_NAME}_PYTHON_DIR}/[!_]*.py*")
  if(py_scripts)
    # write an __init__ file for this python module (if any python scripts are installed)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py 
        "\"\"\"Python module to configure the LDMX module ${MODULE_NAME}\"\"\"")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION ${PYTHON_INSTALL_DIR})
    foreach(pyscript ${py_scripts})
      string(REPLACE ".in" "" script_output ${pyscript})
      get_filename_component(script_output ${script_output} NAME)
      configure_file(${pyscript} ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output})
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output} DESTINATION ${PYTHON_INSTALL_DIR})
    endforeach()
  endif()

  # install anything in the data directory to data/${MODULE}
  file(GLOB data_files "${${MODULE_NAME}_DATA_DIR}/*")
  foreach(data_file ${data_files})
    install(FILES ${data_file} DESTINATION data/${MODULE_NAME} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
  endforeach()

  # print debug info 
  if (MODULE_DEBUG)
    message("MODULE_NAME='${MODULE_NAME}'")
    message("MODULE_DEPENDENCIES='${MODULE_DEPENDENCIES}'")
    message("MODULE_EXTERNAL_DEPENDENCIES='${MODULE_EXTERNAL_DEPENDENCIES}'")
    message("MODULE_INCLUDE_DIR='${MODULE_INCLUDE_DIR}'")
    message("MODULE_SOURCE_DIR='${MODULE_SOURCE_DIR}'")
    message("MODULE_EXTRA_SOURCES='${MODULE_EXTRA_SOURCES}'")
    message("MODULE_EXTRA_LINK_LIBRARIES='${MODULE_EXTRA_LINK_LIBRARIES}'")
    message("MODULE_EXECUTABLES='${MODULE_EXECUTABLES}'")
    message("MODULE_LIBRARIES='${MODULE_LIBRARIES}'")
    message("${MODULE_NAME}_BIN_LIBRARIES='${${MODULE_NAME}_BIN_LIBRARIES}'")
    message("installing python scripts: ${py_scripts}")
    message("installing and configuring python scripts: ${need_config_py_scripts}")
    message("installing data files: ${data_files}")
    message("")
  endif()
  
endmacro(MODULE)
