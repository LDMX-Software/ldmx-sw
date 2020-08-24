

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
    set(link_against_dictionary "NO")
    foreach(src ${SRC_FILES})
        list(FIND event_sources ${src} index)
        if(${index} GREATER 0)
            #found ==> is an event passenger
            list(REMOVE_ITEM SRC_FILES ${src})
            set(link_against_dictionary "YES")
        endif()
    endforeach()

    # Create the SimCore shared library
    add_library(${setup_library_name} SHARED ${SRC_FILES})

    # Setup the include directories 
    target_include_directories(${setup_library_name} PUBLIC ${PROJECT_SOURCE_DIR}/include)

    # Setup the targets to link against 
    if(${link_against_dictionary} STREQUAL "YES")
        list(APPEND setup_library_dependencies "DARK::Event")
    endif()
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

macro(declare_event_object)

    # declare a class to be an event bus passenger
    #   class is the name of the class (required)
    #   namespace is the namespace the class is defined in (default "ldmx")
    #   if collection is defined, then the event bus passenger is added as a vector
    #       e.g. collection "ON"
    #   if map is defined, then the event bus passenger is the value and the key is
    #       what map is defined as (e.g. map "int")
    #   if just_dictionary is defined, then the class is not added to the list
    #       of classes that can be added to the event bus
    #       e.g. just_dictionary "ON"

    set(oneValueArgs 
        class namespace collection map just_dictionary
        )
    cmake_parse_arguments(declare_event_object "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    set(event_headers ${event_headers} 
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/${declare_event_object_class}.h
        CACHE INTERNAL "event_headers" )

    if(EXISTS ${PROJECT_SOURCE_DIR}/src/${declare_event_object_class}.cxx)
        set(event_sources ${event_sources} 
            ${PROJECT_SOURCE_DIR}/src/${declare_event_object_class}.cxx
            CACHE INTERNAL "event_sources" )
    endif()

    set(full_class_name "ldmx::${declare_event_object_class}")
    if(DEFINED declare_event_object_namespace)
        set(full_class_name "${declare_event_object_namespace}::${declare_event_object_class}")
    endif()

    set(dict_classes ${dict_classes} ${full_class_name}
        CACHE INTERNAL "dict_classes")

    if(DEFINED declare_event_object_collection)
        set(event_classes ${event_classes} 
            "std::vector<${full_class_name}>"
            CACHE INTERNAL "event_classes" )
        set(dict_classes ${dict_classes}
            "std::vector<${full_class_name}>"
            CACHE INTERNAL "dict_classes")
    elseif(DEFINED declare_event_object_map)
        set(event_classes ${event_classes}
            "std::map<${declare_event_object_map},${full_class_name}>"
            CACHE INTERNAL "event_classes" )
        set(dict_classes ${dict_classes}
            "std::map<${declare_event_object_map},${full_class_name}>"
            CACHE INTERNAL "dict_classes")
    elseif(NOT DEFINED declare_event_object_just_dictionary)
        set(event_classes ${event_classes} 
            ${full_class_name}
            CACHE INTERNAL "event_classes" )
    endif()

endmacro()

macro(setup_dictionary)

    # Setup a ROOT Dictionary for this build configuration
    #   We assume that all declare_event_object calls are done

    # Arguments

    set(oneValueArgs)
    cmake_parse_arguments(setup_dictionary "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )


    message("Writing ROOT Dictionary for ${PROJECT_NAME}")

    list(REMOVE_DUPLICATES event_classes)
    list(REMOVE_DUPLICATES event_headers)
    list(REMOVE_DUPLICATES event_sources)

    message("Event Classes:'${event_classes}'")
    message("Event Headers:'${event_headers}'")
    
    # Write the EventLinkDef file to be used in the dictionary generation
    set(event_link_def_file ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/EventLinkDef.h)
    file(WRITE ${event_link_def_file}
        "/**\n * @file EventLinkDef.h\n * Automatically generated pre-processor commands for configuring ROOT Event dictionary.\n */\n\n#ifdef __CINT__\n\n#pragma link off all globals;\n#pragma link off all classes;\n#pragma link off all functions;\n#pragma link C++ nestedclass;\n#pragma link C++ nestedtypedef;\n#pragma link C++ namespace ldmx;\n#pragma link C++ defined_in namespace ldmx;\n\n"
        )
    foreach(class ${dict_classes})
        file(APPEND ${event_link_def_file}
            "#pragma link C++ class ${class}+;\n"
            )
    endforeach()
    file(APPEND ${event_link_def_file} "\n#endif")

    message("Wrote ${event_link_def_file}")
    
    # Write the EventDef file to be used in the dictionary generation
    set(event_def_file ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/EventDef.h)
    file(WRITE ${event_def_file}
        "/**\n * @file EventDef.h\n * @brief Automatically generated event bus passenger header.\n */\n"
        )
    foreach(event_header ${event_headers})
        string(REGEX REPLACE "^.*include\/" "" trimmed_header ${event_header})
        file(APPEND ${event_def_file}
            "#include \"${trimmed_header}\"\n"
            )
    endforeach()
    
    file(APPEND ${event_def_file}
        "\n#include <variant>\n\nnamespace ldmx {\n\n    /**\n     * @type EventBusPassenger\n     * Definition of allowed types in the event bus to be handled under one name.\n     */\n    typedef std::variant<\n"
        )
    list(LENGTH event_classes num_event_classes)
    math(EXPR num_event_classes "${num_event_classes}-1")
    foreach(index RANGE ${num_event_classes})
        list(GET event_classes ${index} event_class)
        if(${index} EQUAL ${num_event_classes})
            file(APPEND ${event_def_file} "        ${event_class}\n")
        else()
            file(APPEND ${event_def_file} "        ${event_class},\n")
        endif()
    endforeach()
    file(APPEND ${event_def_file} "    > EventBusPassenger;\n}")
    
    message("Wrote ${event_def_file}")

    # Search and configure ROOT
    find_package(ROOT 6.16 CONFIG REQUIRED)
    
    # Generate the ROOT dictionary.  The following allows the use of the macro 
    # used to generate the dictionary. 
    include("${ROOT_DIR}/RootMacros.cmake")

    message("Imported ROOT")
    
    # Unfortunately, the Event headers need to be included globably for the 
    # dictionary to be generated correctly. 
    foreach(header ${event_headers})
        string(REGEX REPLACE "include.*$" "include" header_dir ${header})
        include_directories(${header_dir})
    endforeach()
    
    message("Included Event Header directories")

    file(COPY ${event_def_file} ${event_link_def_file}
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})

    # Generate the ROOT dictionary
    root_generate_dictionary(EventDict 
        ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}/EventDef.h
        LINKDEF ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}/EventLinkDef.h
    )
    
    message("Generated ROOT Dictionary")

    # Install ROOT pcm file
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDict_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

    message("Installed ROOT Dictioanry pcm File")

    # Create the Event shared library
    list(APPEND event_sources "EventDict.cxx")
    message("Event Sources:'${event_sources}'")

    add_library(Event SHARED ${event_sources})

    # Setup the include directories 
    target_include_directories(Event PUBLIC ROOT::Core)
    foreach(header ${event_headers})
        string(REGEX REPLACE "include.*$" "include" header_dir ${header})
        target_include_directories(Event PUBLIC ${header_dir})
    endforeach()
    target_link_libraries(Event PUBLIC ROOT::Core)

    # Define an alias. This is used to create the imported target.
    add_library(DARK::Event ALIAS Event) 

    # Install the libraries and headers
    install(TARGETS Event
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    )

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
