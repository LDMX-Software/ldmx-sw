

include(CMakeParseArguments)

# Define some colors. These are used to colorize CMake's user output
if (NOT WIN32)
    string(ASCII 27 esc)
    set(color_reset "${esc}[m")
    set(bold_yellow "${esc}[1;33m")
    set(green       "${esc}[32m")
    set(bold_red    "${esc}[1;31m")
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
            INTERFACE_LINK_LIBRARIES "${Geant4_LIBRARIES}"
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

    set(oneValueArgs name python_install_path)
    set(multiValueArgs dependencies sources)
    cmake_parse_arguments(setup_library "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    # Find all of the source files we want to add to the library
    if (NOT setup_library_sources)
        file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)
    else()
        set(SRC_FILES ${setup_library_sources})
    endif()

    #NOT SURE IF NEEDED
    #   do I need to remove the source files that were compiled into the 
    #   event bus dictionary? Or is it okay for them to be compiled twice?
    #set(link_against_dictionary "NO")
    #foreach(src ${SRC_FILES})
    #    list(FIND event_sources ${src} index)
    #    if(${index} GREATER 0)
            #found ==> is an event passenger
            #        list(REMOVE_ITEM SRC_FILES ${src})
            #set(link_against_dictionary "YES")
            #endif()
            #endforeach()

    # Create the SimCore shared library
    add_library(${setup_library_name} SHARED ${SRC_FILES})

    # Setup the include directories 
    target_include_directories(${setup_library_name} PUBLIC ${PROJECT_SOURCE_DIR}/include)

    # Setup the targets to link against 
    #if(${link_against_dictionary} STREQUAL "YES")
    #    list(APPEND setup_library_dependencies "DARK::Event")
    #endif()
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
       
        # Install the python modules
        file(GLOB py_scripts CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/python/*.py)
        foreach(pyscript ${py_scripts})
            string(REPLACE ".in" "" script_output ${pyscript})
            get_filename_component(script_output ${script_output} NAME)
            configure_file(${pyscript} ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output})
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/${script_output} DESTINATION ${setup_library_python_install_path}/${setup_library_name})
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

function(register_event_object)

    set(oneValueArgs namespace class type)
    cmake_parse_arguments(register_event_object "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    # Start by checking if the class that is being registered exists
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/src/${register_event_object_class}.cxx)
        message(FATAL_ERROR "Trying to register class ${register_event_object_class} that doesn't exist.")
    endif()

    get_filename_component(header_dir ${PROJECT_SOURCE_DIR} NAME)
    set(header ${header_dir}/${register_event_object_class}.h)

    if(DEFINED register_event_object_namespace)
        STRING(CONCAT register_event_object_class
                      ${register_event_object_namespace} "::" 
                      ${register_event_object_class})
    endif()

    # Only register objects that haven't already been registered.
    if(register_event_object_class IN_LIST dict)
        return()
    endif()

    set(event_headers ${event_headers} ${header} CACHE INTERNAL "event_headers")

    # If the passenger type was not specified, then add the class to the event
    # bus stand alone.
    if(NOT DEFINED ${register_event_object_type})
        message(STATUS ${register_event_object_type})
        set(bus_passengers ${bus_passengers} ${register_event_object_class} CACHE INTERNAL "bus_passengers")
        set(dict ${dict} ${register_event_object_class} CACHE INTERNAL "dict")
    elseif(register_event_object_type STREQUAL "collection")
        set(bus_passengers ${bus_passengers} 
            "std::vector< ${register_event_object_class} >" CACHE INTERNAL "bus_passengers")
        set(dict ${dict} ${register_event_object_class} 
            "std::vector< ${register_event_object_class} >" CACHE INTERNAL "dict")
    elseif(register_event_object_type STREQUAL "map")
    elseif(register_event_object_type STREQUAL "dic_only")
        set(dict ${dict} ${register_event_object_class} CACHE INTERNAL "dict")
    endif()

endfunction()

macro(build_event_bus)

    get_filename_component(header_dir ${PROJECT_SOURCE_DIR} NAME)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/include/${header_dir}/EventDef.h)
        
        set(file_path ${PROJECT_SOURCE_DIR}/include/${header_dir}/EventDef.h)

        foreach(header ${event_headers})
            file(APPEND ${file_path} "#include \"${header}\"\n")
        endforeach()
        
        list(LENGTH bus_passengers passenger_count)
        MATH(EXPR last_passenger_index "${passenger_count} - 1")
        list(GET bus_passengers ${last_passenger_index} last_passenger)
        list(REMOVE_AT bus_passengers ${last_passenger_index})

        file(APPEND ${file_path} "\n#include <variant>\n\n")
        file(APPEND ${file_path} "typedef std::variant<\n")
        foreach(passenger ${bus_passengers})
            file(APPEND ${file_path} "    ${passenger},\n")
        endforeach()
        
        file(APPEND ${file_path} "    ${last_passenger}\n")
        file(APPEND ${file_path} "> EventBusPassenger;")

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

macro(install_external)
endmacro()
