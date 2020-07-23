

include(CMakeParseArguments)

#
# Process the Geant4 targets so they are modern cmake compatible.
#
macro(setup_geant4_target)

    # Search for Geant4 and load its settings
    find_package(Geant4 REQUIRED gdml ui_all vis_all)

    # Create an imported Geant4 target if it hasn't been done yet.
    if( NOT TARGET Geant4::Interface) 
    
        #Geant4_DEFINITIONS already include -D, this leads to the error 
        # <command-line>:0:1: error: macro names must be identifiers
        # if not removed.
        SET(G4_DEF_TEMP "")
        foreach(def ${Geant4_DEFINITIONS})
            string(REPLACE "-D" "" def ${def})
            LIST(APPEND G4_DEF_TEMP ${def})
        endforeach()
        SET(LDMX_Geant4_DEFINITIONS ${G4_DEF_TEMP})
        UNSET(G4_DEF_TEMP)

        # Create the Geant4 target
        add_library(Geant4::Interface INTERFACE IMPORTED GLOBAL)

        # Set the target properties
        SET_TARGET_PROPERTIES(Geant4::Interface
            PROPERTIES
            INTERFACE_COMPILE_OPTIONS "${Geant4_Flags}"
            INTERFACE_COMPILE_DEFINITIONS "${LDMX_Geant4_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${Geant4_INCLUDE_DIRS}"
        )

        message(STATUS "Found Geant4 version ${Geant4_VERSION}")
    
    endif()

endmacro()

macro(setup_lcio_target)
    
    # If it doesn't exists, create an imported target for LCIO
    if ( NOT TARGET LCIO::Interface)
    
        # Search for LCIO and load its settings
        find_package(LCIO CONFIG REQUIRED)

        # If the LCIO package can't be found, error out.
        if( NOT LCIO_FOUND)
            message(FATAL_ERROR "Failed to find required dependency LCIO.")
        endif()

        # Create the LCIO target
        add_library(LCIO::Interface SHARED IMPORTED GLOBAL)

        # Set the target properties
        set_target_properties(LCIO::Interface
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LCIO_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${LCIO_LIBRARY_DIRS}/liblcio.so"
        )

        message(STATUS "Found LCIO version ${LCIO_VERSION}")

    endif()

endmacro()

macro(setup_library)

    set(oneValueArgs name)
    set(multiValueArgs dependencies sources)
    cmake_parse_arguments(setup_library "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    # Find all of the source files we want to add to the library
    if (NOT setup_library_sources)
        file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)
    else()
        set(SRC_FILES ${setup_library_sources})
    endif()

    # Create the SimCore shared library
    add_library(${setup_library_name} SHARED ${SRC_FILES})

    # Setup the include directories 
    target_include_directories(${setup_library_name} PUBLIC ${PROJECT_SOURCE_DIR}/include)

    # Setup the targets to link against 
    target_link_libraries(${setup_library_name} PUBLIC ${setup_library_dependencies})

    # Define an alias. This is used to create the imported target.
    add_library(DARK::${setup_library_name} ALIAS ${setup_library_name})

    # Install the libraries and headers
    install(TARGETS ${setup_library_name}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    )
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${setup_library_name}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

    # If the python directory exists, initialize the package and copy over the 
    # python modules.
    if (EXISTS ${PROJECT_SOURCE_DIR}/python)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py 
             "\"\"\"Python module to configure the LDMX module ${setup_library_name}\"\"\"")
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py 
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python/LDMX/${setup_library_name})

        # write an include file for this module
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/include.py
             "\"\"\"Include this module\"\"\"\ndef library() :\n\ \ \ \ \"\"\"Attach the name of ${setup_library_name} library to the process\"\"\"\n\ \ \ \ from LDMX.Framework.ldmxcfg import Process\n\ \ \ \ Process.addLibrary('${CMAKE_INSTALL_PREFIX}/lib/lib${setup_library_name}.so')")
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/include.py DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python/LDMX/${setup_library_name})
        # install python scripts
        file(GLOB py_scripts CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/python/*.py)
        foreach(pyscript ${py_scripts})
            string(REPLACE ".in" "" script_output ${pyscript})
            get_filename_component(script_output ${script_output} NAME)
            configure_file(${pyscript} ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output})
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python/LDMX/${setup_library_name})
        endforeach()

    endif()

    # If the data directory exists, install it to the data directory
    if (EXISTS ${PROJECT_SOURCE_DIR}/data)
        file(GLOB data_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/data/*)
        foreach(data_file ${data_files})
            install(FILES ${data_file} DESTINATION ${CMAKE_INSTALL_PREFIX}/data/${setup_library_name})
        endforeach()
    endif()

endmacro()

macro(setup_test)

    set(multiValueArgs dependencies)
    cmake_parse_arguments(setup_test "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    # Find all the test
    file(GLOB src_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/test/*.cxx)

    # Add all test to the global list of test sources 
    set(test_sources ${test_sources} ${src_files} CACHE INTERNAL "test_sources")

    # Add all of the dependencies to the global list of dependencies 
    set(test_dep ${test_dep} ${setup_test_dependencies} CACHE INTERNAL "test_dep")

endmacro()

macro(build_test)

    # If test have been enabled, attempt to find catch.  If catch hasn't 
    # found, it will be downloaded locally.
    find_package(Catch2 2.13.0)

    # Create the Catch2 main exectuable if it doesn't exist
    if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/external/catch/run_test.cxx)
    
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/external/catch/run_test.cxx
             "#define CATCH_CONFIG_MAIN\n#include \"catch.hpp\""
        )
        
        message(STATUS "Writing Catch2 main to: ${CMAKE_CURRENT_BINARY_DIR}/external/catch/run_test.cxx")
    endif()

    # Add the executable to run all test.  test_sources is a cached variable 
    # that contains the test from the different modules.  Each of the modules
    # needs to setup the test they want to run.
    add_executable(run_test ${CMAKE_CURRENT_BINARY_DIR}/external/catch/run_test.cxx ${test_sources})
    target_include_directories(run_test PRIVATE ${CATCH2_INCLUDE_DIR})
    target_link_libraries(run_test PRIVATE Catch2::Interface ${test_dep})

    # Install the run_test  executable
    install(TARGETS run_test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

endmacro()
