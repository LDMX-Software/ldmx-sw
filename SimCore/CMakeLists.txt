# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  SimCore
  VERSION 1.4.0
  DESCRIPTION "Core classes needed to run the Simulator."
  LANGUAGES CXX)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

# Search for Boost
find_package(Boost REQUIRED COMPONENTS log)

include_directories( /usr/local/include/GENIE )

option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

  install(FILES /usr/local/lib/libHepMC3rootIO_rdict.pcm DESTINATION lib)

  register_event_object(module_path "SimCore/Event" namespace "ldmx" 
                        class "SimCalorimeterHit" type "collection")
  register_event_object(module_path "SimCore/Event" namespace "ldmx"
                        class "SimTrackerHit" type "collection")
  register_event_object(module_path "SimCore/Event" namespace "ldmx"
                        class "SimParticle" type "map" key "int")
  register_event_object(module_path "SimCore/Event" namespace "ldmx"
      class "EventWeights")

  register_event_object(module_path "SimCore/Event" namespace "ldmx"
      class "HepMC3GenEvent" type "collection")

  # Generate the files needed to build the event classes.
  setup_library(module SimCore name Event
                dependencies ROOT::Core ${GENIE_LIBS} HepMC3 HepMC3rootIO
                register_target
		include )

  return()

endif()

# Configure Geant4
setup_geant4_target()

#Configure Genie
setup_genie_target()

# include dark brem simulation
add_subdirectory(G4DarkBreM)

setup_library(module SimCore
              name PhotoNuclearModels
              dependencies Geant4::Interface
                           Framework::Configure)

# Set the LHE Reading Library
setup_library(module SimCore 
              name LHE
              dependencies Framework::Exception
                           Geant4::Interface)

# Library of our interactions with G4
setup_library(module SimCore
              name G4User
              dependencies Framework::Framework
                           Geant4::Interface)

# Setup the biasing operators library
setup_library(module SimCore
              name BiasOperators
              dependencies Geant4::Interface
                           Framework::Configure
                           Framework::Framework
                           G4DarkBreM)

# Primary Generators library
setup_library(module SimCore
              name Generators
              dependencies Geant4::Interface
                           Framework::Configure
                           Framework::Framework
                           SimCore::Event
                           SimCore::LHE
                           ${GENIE_LIBS}
                           HepMC3)

# Reweight Library
setup_library(module SimCore
              name Reweight
              dependencies SimCore::Generators
	                         SimCore::Event
                           Framework::Configure
			                     Framework::Framework
                           ${GENIE_LIBS}
			                     HepMC3)

# Sensitive Detectors library
setup_library(module SimCore
              name SDs
              dependencies Geant4::Interface
                           Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr
                           SimCore::G4User)

setup_library(module SimCore
              name Geo
              dependencies Geant4::Interface
                           Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr)

# Setup the library
setup_library(module SimCore
              dependencies Geant4::Interface
                           ROOT::Physics
                           ROOT::MathMore
                           HepMC3
                           Framework::Configure
                           Framework::Framework
                           SimCore::Geo
                           SimCore::G4User
                           SimCore::PhotoNuclearModels
                           SimCore::BiasOperators
                           SimCore::Generators
			                     SimCore::Reweight
                           SimCore::SDs)
			   

# Set some target properties
set_target_properties(SimCore
                      PROPERTIES CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)

setup_python(package_name LDMX/SimCore)

# run all *.py files in test during testing
setup_test(config_dir test)

# add visualization executable
add_executable(g4-vis ${PROJECT_SOURCE_DIR}/src/SimCore/g4_vis.cxx)
target_link_libraries(g4-vis PRIVATE Geant4::Interface SimCore::SimCore)
install(TARGETS g4-vis DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
