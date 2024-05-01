include(CMakeParseArguments)
include(Sanitizers)
include(CompilerWarnings)
include(InterProceduralOptimization)
include(StaticAnalysis)
# Define some colors. These are used to colorize CMake's user output
if(NOT WIN32)
  string(ASCII 27 esc)
  set(color_reset "${esc}[m")
  set(bold_yellow "${esc}[1;33m")
  set(green "${esc}[32m")
  set(bold_red "${esc}[1;31m")
endif()

# Override messages and add color
function(message)
  list(GET ARGV 0 message_type)
  if(message_type STREQUAL FATAL_ERROR OR message_type STREQUAL SEND_ERROR)
    list(REMOVE_AT ARGV 0)
    _message("${bold_red}[ ERROR ]: ${ARGV}${color_reset}")
  elseif(message_type STREQUAL WARNING OR message_type STREQUAL AUTHOR_WARNING)
    list(REMOVE_AT ARGV 0)
    _message("${bold_yellow}[ WARNING ]: ${ARGV}${color_reset}")
  elseif(message_type STREQUAL STATUS)
    list(REMOVE_AT ARGV 0)
    _message("${green}[ INFO ]: ${ARGV}${color_reset}")
  else()
    _message("${green} ${ARGV} ${color_reset}")
  endif()
endfunction()

#
# Process the Geant4 targets so they are modern cmake compatible.
#
macro(setup_geant4_target)

  # Search for Geant4 and load its settings
  find_package(Geant4 REQUIRED gdml ui_all vis_all)

  # Create an imported Geant4 target if it hasn't been done yet.
  if(NOT TARGET Geant4::Interface)

    # Geant4_DEFINITIONS already include -D, this leads to the error
    # <command-line>:0:1: error: macro names must be identifiers if not removed.
    set(G4_DEF_TEMP "")
    foreach(def ${Geant4_DEFINITIONS})
      string(REPLACE "-D" "" def ${def})
      list(APPEND G4_DEF_TEMP ${def})
    endforeach()
    set(LDMX_Geant4_DEFINITIONS ${G4_DEF_TEMP})
    unset(G4_DEF_TEMP)

    # Create the Geant4 target
    add_library(Geant4::Interface INTERFACE IMPORTED GLOBAL)

    # Set the target properties
    set_target_properties(
      Geant4::Interface
      PROPERTIES INTERFACE_LINK_LIBRARIES "${Geant4_LIBRARIES}"
                 INTERFACE_COMPILE_OPTIONS "${Geant4_Flags}"
                 INTERFACE_COMPILE_DEFINITIONS "${LDMX_Geant4_DEFINITIONS}"
                 INTERFACE_INCLUDE_DIRECTORIES "${Geant4_INCLUDE_DIRS}")

    message(STATUS "Found Geant4 version ${Geant4_VERSION}")

  endif()

endmacro()

macro(setup_lcio_target)

  # If it doesn't exists, create an imported target for LCIO
  if(NOT TARGET LCIO::Interface)

    # Search for LCIO and load its settings
    find_package(LCIO CONFIG REQUIRED)

    # If the LCIO package can't be found, error out.
    if(NOT LCIO_FOUND)
      message(FATAL_ERROR "Failed to find required dependency LCIO.")
    endif()

    # Create the LCIO target
    add_library(LCIO::Interface SHARED IMPORTED GLOBAL)

    # Set the target properties
    set_target_properties(
      LCIO::Interface
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LCIO_INCLUDE_DIRS}"
                 IMPORTED_LOCATION "${LCIO_LIBRARY_DIRS}/liblcio.so")

    message(STATUS "Found LCIO version ${LCIO_VERSION}")

  endif()

endmacro()

macro(setup_library)

  set(options interface register_target)
  set(oneValueArgs module name)
  set(multiValueArgs dependencies sources)
  cmake_parse_arguments(setup_library "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Build the library name and source path
  set(library_name "${setup_library_module}")
  set(src_path "${PROJECT_SOURCE_DIR}/src/${setup_library_module}")
  set(include_path "include/${setup_library_module}")
  if(setup_library_name)
    set(library_name "${setup_library_module}_${setup_library_name}")
    set(src_path "${src_path}/${setup_library_name}")
    set(include_path "${include_path}/${setup_library_name}")
  endif()

  # If not an interface, find all of the source files we want to add to the
  # library.
  if(NOT setup_library_interface)
    if(NOT setup_library_sources)
      file(GLOB SRC_FILES CONFIGURE_DEPENDS ${src_path}/[a-zA-Z]*.cxx)
    else()
      set(SRC_FILES ${setup_library_sources})
    endif()

    # Create the SimCore shared library
    add_library(${library_name} SHARED ${SRC_FILES})
  else()
    add_library(${library_name} INTERFACE)
  endif()

  # Setup the include directories
  if(setup_library_interface)
    target_include_directories(${library_name}
                               INTERFACE ${PROJECT_SOURCE_DIR}/include)
  else()
    target_include_directories(${library_name}
                               PUBLIC ${PROJECT_SOURCE_DIR}/include)
  endif()

  # Setup the targets to link against
  target_link_libraries(${library_name} PUBLIC ${setup_library_dependencies})
  enable_sanitizers(${library_name})
  enable_compiler_warnings(${library_name})
  enable_ipo(${library_name})
  enable_clang_tidy(${library_name} ${WARNINGS_AS_ERRORS})

  # Define an alias. This is used to create the imported target.
  set(alias "${setup_library_module}::${setup_library_module}")
  if(setup_library_name)
    set(alias "${setup_library_module}::${setup_library_name}")
  endif()
  add_library(${alias} ALIAS ${library_name})

  if(setup_library_register_target)
    set(registered_targets
        ${registered_targets} ${alias}
        CACHE INTERNAL "registered_targets")
  endif()

  # Install the libraries and headers
  install(TARGETS ${library_name}
          LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/${include_path}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

endmacro()

macro(setup_python)

  set(oneValueArgs package_name)
  cmake_parse_arguments(setup_python "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # If the python directory exists, initialize the package and copy over the
  # python modules.
  if(EXISTS ${PROJECT_SOURCE_DIR}/python)

    # Get a list of all python files inside the python package
    file(GLOB py_scripts CONFIGURE_DEPENDS
         ${PROJECT_SOURCE_DIR}/python/[a-zA-Z]*.py
         ${PROJECT_SOURCE_DIR}/python/[a-zA-Z]*.py.in)

    foreach(pyscript ${py_scripts})

      # If a filename has a '.in' extension, remove it.  The '.in' extension is
      # used to denote files that have variables that will be substituded by the
      # configure_file macro.
      string(REPLACE ".in" "" script_output ${pyscript})

      # GLOB returns the full path to the file.  We also need the filename so
      # it's new location can be specified.
      get_filename_component(script_output ${script_output} NAME)

      # Copy the file from its original location to the bin directory.  This
      # will also replace all cmake variables within the files.
      configure_file(${pyscript}
                     ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output})

      # Install the files to the given path
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/python/${setup_python_package_name})
    endforeach()

  endif()

endmacro()

macro(setup_data)

  set(oneValueArgs module)
  cmake_parse_arguments(setup_data "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # If the data directory exists, install it to the data directory
  if(EXISTS ${PROJECT_SOURCE_DIR}/data)
    file(GLOB data_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/data/*)
    foreach(data_file ${data_files})
      install(FILES ${data_file}
              DESTINATION ${CMAKE_INSTALL_PREFIX}/data/${setup_data_module})
    endforeach()
  endif()

endmacro()

function(register_event_object)

  set(oneValueArgs module_path namespace class type key)
  cmake_parse_arguments(register_event_object "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Start by checking if the class that is being registered exists
  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/include/${register_event_object_module_path}/${register_event_object_class}.h)
    message(FATAL_ERROR
      "Trying to register class ${register_event_object_class} that doesn't exist.")
  endif()

  set(header
      ${register_event_object_module_path}/${register_event_object_class}.h)

  if(DEFINED register_event_object_namespace)
    string(CONCAT register_event_object_class
                  ${register_event_object_namespace} "::"
                  ${register_event_object_class})
  endif()

  # Only register objects that haven't already been registered.
  if(register_event_object_class IN_LIST dict)
    return()
  endif()

  set(dict
      ${dict} ${register_event_object_class}
      CACHE INTERNAL "dict")

  set(event_headers
      ${event_headers} ${header}
      CACHE INTERNAL "event_headers")

  set(namespaces
      ${namespaces} ${register_event_object_namespace}
      CACHE INTERNAL "namespaces")

  if(NOT ${PROJECT_SOURCE_DIR}/include IN_LIST include_paths)
    set(include_paths
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>" ${include_paths}
        CACHE INTERNAL "include_paths")
  endif()

  if(register_event_object_type STREQUAL "collection")
    set(dict
        ${dict} 
        "std::vector< ${register_event_object_class} >"
        CACHE INTERNAL "dict")
  elseif(register_event_object_type STREQUAL "map")
    set(dict
        ${dict}
        "std::map< ${register_event_object_key}, ${register_event_object_class} >"
        CACHE INTERNAL "dict")
  elseif(DEFINED register_event_object_type)
    message(
      FATAL_ERROR
        "Trying to register object with invalid type ${register_event_object_type}"
    )
  endif()

endfunction()

macro(setup_test)

  set(oneValueArgs config_dir)
  set(multiValueArgs dependencies sources configs)
  cmake_parse_arguments(setup_test "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Find all the test
  if(DEFINED setup_test_sources)
    set(src_files ${setup_test_sources})
  else()
    file(GLOB src_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/test/[a-zA-Z]*.cxx)
  endif()

  get_filename_component(module ${PROJECT_SOURCE_DIR} NAME)

  if (src_files)
    # Add all test to the global list of test sources
    set(test_sources
        ${test_sources} ${src_files}
        CACHE INTERNAL "test_sources")
  
    # Add all of the dependencies to the global list of dependencies
    set(test_dep
        ${test_dep} ${setup_test_dependencies}
        CACHE INTERNAL "test_dep")
  
    # Add the module to the list of tags
    set(test_modules
        ${test_modules} ${module}
        CACHE INTERNAL "test_modules")
  endif()

  if(DEFINED setup_test_configs)
    set(new_test_configs ${setup_test_configs})
  elseif(DEFINED setup_test_config_dir)
    file(GLOB new_test_configs CONFIGURE ${setup_test_config_dir}/[a-zA-Z]*.py)
  endif()

  foreach(new_config ${new_test_configs})
    set(test_configs
        ${test_configs} "${module}:${new_config}"
        CACHE INTERNAL "test_configs"
        )
  endforeach()

endmacro()

macro(build_test)

  enable_testing()

  # If test have been enabled, attempt to find catch.  If catch hasn't found, it
  # will be downloaded locally.
  find_package(Catch2 QUIET)

  if (NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.3.1.tar.gz
        URL_HASH MD5=5cdc99f93e0b709936eb5af973df2a5c
    )
    FetchContent_MakeAvailable(Catch2)
  endif()

  # Add the executable to run all test.  test_sources is a cached variable that
  # contains the test from the different modules.  Each of the modules needs to
  # setup the test they want to run.
  add_executable(run_test ${test_sources})
  target_link_libraries(run_test PRIVATE Catch2::Catch2WithMain ${test_dep})

  foreach(entry ${test_modules})
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${entry}/test)
    file(GLOB test_contents ${PROJECT_SOURCE_DIR}/${entry}/test/**)
    foreach(file ${test_contents})
      get_filename_component(filename ${file} NAME)
      file(CREATE_LINK ${file} ${CMAKE_BINARY_DIR}/${entry}/test/${filename} SYMBOLIC)
    endforeach()
    add_test(
      NAME ${entry} 
      COMMAND run_test "[${entry}]" 
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${entry}/test
      )
  endforeach()

  foreach(config ${test_configs})
    string(REPLACE ":" ";" config_list ${config})
    list(GET config_list 0 module)
    list(GET config_list 1 config_filepath)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${module}/test)
    get_filename_component(config_name ${config_filepath} NAME)
    add_test(
        NAME "Run_${module}_${config_name}"
        COMMAND fire ${config_filepath}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${module}/test
        )
  endforeach()
endmacro()

macro(clear_cache_variables)
  unset(registered_targets CACHE)
  unset(dict CACHE)
  unset(event_headers CACHE)
  unset(test_sources CACHE)
  unset(test_dep CACHE)
  unset(test_modules CACHE)
  unset(namespaces CACHE)
  unset(test_configs CACHE)
endmacro()
