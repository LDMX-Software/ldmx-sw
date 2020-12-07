# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  SimCore
  VERSION 2.1.0
  DESCRIPTION "Core classes needed to run the Simulator."
  LANGUAGES CXX)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

# Search for Boost
find_package(Boost REQUIRED
             COMPONENTS log)

option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

  register_event_object(module_path "SimCore/Event" namespace "ldmx" 
                        class "SimCalorimeterHit" type "collection")
  register_event_object(module_path "SimCore/Event" namespace "ldmx"
                        class "SimTrackerHit" type "collection")
  register_event_object(module_path "SimCore/Event" namespace "ldmx"
                        class "SimParticle" type "map" key "int")

  # Generate the files needed to build the event classes.
  setup_library(module SimCore name Event
                dependencies ROOT::Core
                register_target)

  return()

endif()

# Configure Geant4
setup_geant4_target()


# Get a list of all of the source files.
file(GLOB SRC_FILES CONFIGURE_DEPDENDS ${PROJECT_SOURCE_DIR}/src/SimCore/Persist/[a-zA-Z]*.cxx
                                       ${PROJECT_SOURCE_DIR}/src/SimCore/DarkBrem/[a-zA-Z]*.cxx
                                       ${PROJECT_SOURCE_DIR}/src/SimCore/[a-zA-Z]*.cxx)

# Setup the library
setup_library(module SimCore
              dependencies Geant4::Interface
                           ROOT::Physics
                           Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr
                           Boost::log
                           "${registered_targets}"
              sources ${SRC_FILES})

# Setup the biasing operators library
setup_library(module SimCore
              name BiasOperators
              dependencies SimCore::SimCore)

# Set some target properties
set_target_properties(SimCore
                      PROPERTIES CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)

# Unpack the example dark brem vertex library (or libraries)
file(GLOB vertex_libraries data/*.tar.gz)

# set installation location of dark brem vertex libraries
#   also used when configuring the makePath.py module
set(DB_VERTEX_LIB_INSTALL ${CMAKE_INSTALL_PREFIX}/data/db_vertex_libraries)
if(NOT EXISTS ${DB_VERTEX_LIB_INSTALL})
    #assume new build ==> unpack vertex libraries
    message(STATUS "Unpacking example dark brem vertex libraries.")
    file(MAKE_DIRECTORY ${DB_VERTEX_LIB_INSTALL})
    foreach(library_path ${vertex_libraries})
        execute_process(COMMAND 
            tar -C ${DB_VERTEX_LIB_INSTALL} --skip-old-files -zxf ${library_path})
    endforeach()
endif()

setup_python(package_name ${PYTHON_PACKAGE_NAME}/SimCore)

# Add compiled tests
configure_file(${PROJECT_SOURCE_DIR}/test/DarkBremProcessTest.cxx.in 
               ${CMAKE_CURRENT_BINARY_DIR}/test/DarkBremProcessTest.cxx)
setup_test(dependencies SimCore::SimCore Geant4::Interface)

# Configure the executable so it knows the installation path of the db vertex libraries
configure_file(${PROJECT_SOURCE_DIR}/src/SimCore/DarkBrem/print_dark_brem_xsec_table.cxx.in
               ${CMAKE_CURRENT_BINARY_DIR}/bin/print_dark_brem_xsec_table.cxx)
add_executable(print-dark-brem-xsec-table 
               ${CMAKE_CURRENT_BINARY_DIR}/bin/print_dark_brem_xsec_table.cxx)

# Link to the Framework library
target_link_libraries(print-dark-brem-xsec-table 
                      PRIVATE Geant4::Interface 
                              SimCore::SimCore 
                              Framework::Framework)

# Install the fire executable
install(TARGETS print-dark-brem-xsec-table 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

