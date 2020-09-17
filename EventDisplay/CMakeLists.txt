
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(EventDisplay VERSION 2.1.0
                     DESCRIPTION "Event display module." 
                     LANGUAGES CXX
)

# Search for the Python3 library
find_package (Python3 COMPONENTS Interpreter Development)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Generate the ROOT dictionary.  The following allows the use of the macro 
# used to generate the dictionary. 
include("${ROOT_DIR}/RootMacros.cmake")

# Unfortunately, the Event headers need to be included globably for the 
# dictionary to be generated correctly.
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LDMX_SW_SOURCE_DIR}/DetDescr/include)
include_directories(${include_paths})

# Generate the ROOT dictionary
root_generate_dictionary(EventDisplayDic ${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayDef.h 
    LINKDEF ${PROJECT_SOURCE_DIR}/include/EventDisplay/EventDisplayLinkDef.h)

# Install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDisplayDic_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Find all of the source files we want to add to the Event library
file(GLOB EVENT_DISPLAY_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/EventDisplay/*.cxx)

# Create the Event target
add_library(EventDisplay SHARED EventDisplayDic.cxx ${EVENT_DISPLAY_SRC_FILES})

# Setup the include directories 
target_include_directories(EventDisplay PUBLIC ${PROJECT_SOURCE_DIR}/include/)

# Setup the libraries to link against 
target_link_libraries(EventDisplay PUBLIC
    Python3::Python
    ROOT::Core
    ROOT::Rint
    ROOT::Geom
    ROOT::Gui
    ROOT::Eve
    DetDescr::DetDescr
    Framework::Framework
)

# Define an alias 
add_library(DARK::EventDisplay ALIAS EventDisplay)

# Install the libraries and headers
install(TARGETS EventDisplay
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/EventDisplay 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# Add the event display executable
add_executable(ldmx-eve ${PROJECT_SOURCE_DIR}/src/EventDisplay/ldmx_eve.cxx)

# Link to the Framework library
target_link_libraries(ldmx-eve PRIVATE EventDisplay)

# Install the fire executable
install(TARGETS ldmx-eve DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
