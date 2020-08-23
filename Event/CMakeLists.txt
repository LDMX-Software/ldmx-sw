
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Event VERSION 2.1.0
              DESCRIPTION "Event classes used to persist LDMX simulation and physics objects." 
              LANGUAGES CXX
)

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
    ${PROJECT_SOURCE_DIR}/include/Event/Version.h.in
    ${PROJECT_SOURCE_DIR}/include/Event/Version.h
)
install(FILES ${PROJECT_SOURCE_DIR}/include/Event/Version.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Event/)

declare_event_object(
    class "CalorimeterHit"
    module "Event" collection "ON"
    )

declare_event_object(
    class "ClusterAlgoResult"
    module "Event"
    )

declare_event_object(
    class "EventHeader"
    module "Event"
    )

declare_event_object(
    class "EcalCluster"
    module "Event" collection "ON"
    )

declare_event_object(
    class "EcalHit"
    module "Event" collection "ON"
    )

declare_event_object(
    class "EcalDigiCollection"
    module "Event"
    )

declare_event_object(
    class "EcalVetoResult"
    module "Event"
    )

declare_event_object(
    class "FindableTrackResult"
    module "Event" collection "ON"
    )

declare_event_object(
    class "HcalHit"
    module "Event" collection "ON"
    )

declare_event_object(
    class "HcalVetoResult"
    module "Event"
    )

declare_event_object(
    class "SimCalorimeterHit"
    module "Event" collection "ON"
    )

declare_event_object(
    class "SimTrackerHit"
    module "Event" collection "ON"
    )

declare_event_object(
    class "SiStripHit"
    module "Event" collection "ON"
    )

declare_event_object(
    class "SimParticle"
    module "Event" map "int"
    )

declare_event_object(
    class "TrigScintHit"
    module "Event" collection "ON"
    )

declare_event_object(
    class "TrackerVetoResult"
    module "Event"
    )

declare_event_object(
    class "TriggerResult"
    module "Event"
    )

# Write the EventLinkDef file to be used in the dictionary generation
set(event_link_def_file ${PROJECT_SOURCE_DIR}/include/Event/EventLinkDef.h)
file(WRITE ${event_link_def_file}
    "/**\n * @file EventLinkDef.h\n * Automatically generated pre-processor commands for configuring ROOT Event dictionary.\n */\n\n#ifdef __CINT__\n\n#pragma link off all globals;\n#pragma link off all classes;\n#pragma link off all functions;\n#pragma link C++ nestedclass;\n#pragma link C++ nestedtypedef;\n#pragma link C++ namespace ldmx;\n#pragma link C++ defined_in namespace ldmx;\n\n"
    )
foreach(event_class ${event_classes})
    file(APPEND ${event_link_def_file}
        "#pragma link C++ class ${event_class}+;\n"
        )
endforeach()
file(APPEND ${event_link_def_file}
    "\n#endif"
    )

# Write the EventDef file to be used in the dictionary generation
set(event_def_file ${PROJECT_SOURCE_DIR}/include/Event/EventDef.h)
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
while(${num_event_classes} GREATER 1)
    list(POP_FRONT ${event_classes} event_class)
    file(APPEND ${event_def_file}
        "        ${event_class},\n"
        )
    list(LENGTH event_classes num_event_classes)
endwhile(${num_event_classes} GREATER 1)

# put the last one in with no comma
list(POP_FRONT ${event_classes} event_class)
file(APPEND ${event_def_file}
    "        ${event_class}\n    > EventBusPassenger;\n}"
    )

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Generate the ROOT dictionary.  The following allows the use of the macro 
# used to generate the dictionary. 
include("${ROOT_DIR}/RootMacros.cmake")

# Unfortunately, the Event headers need to be included globably for the 
# dictionary to be generated correctly. 
foreach(header ${event_headers})
    string(REGEX REPLACE "include.*$" "include" header_dir ${header})
    message("Including: ${header_dir}")
    include_directories(${header_dir})
endforeach()

# Copy the files needed to generate the dictionary over to the installation 
# directory.  This is needed so the dictionary can be built even when the 
# source code is deleted.
file(COPY ${PROJECT_SOURCE_DIR}/include/Event/EventDef.h 
          ${PROJECT_SOURCE_DIR}/include/Event/EventLinkDef.h
     DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Event/
)

# Generate the ROOT dictionary
root_generate_dictionary(EventDic ${CMAKE_INSTALL_PREFIX}/include/Event/EventDef.h
    LINKDEF ${CMAKE_INSTALL_PREFIX}/include/Event/EventLinkDef.h)

# Install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDic_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Need to define the sources here because of the addition of EventDic
#file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cxx)

# Setup the library
setup_library(name Event
              dependencies ROOT::Core
              sources EventDic.cxx ${event_sources}
              python_install_path ${PYTHON_INSTALL_PREFIX}
)

# If this is really needed, import a target instead.
# Add the location of Version.h to the build. Also, set the variable
# EXTRA_INC in the parent scope so the extra include directory can 
# be added to the whole project not just this module.
#include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
#set(EXTRA_INC ${CMAKE_CURRENT_BINARY_DIR}/include/ PARENT_SCOPE)$
