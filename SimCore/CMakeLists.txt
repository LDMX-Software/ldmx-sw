# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  SimCore
  VERSION 1.4.0
  DESCRIPTION "Core classes needed to run the Simulator."
  LANGUAGES CXX)

setup_library(
  module SimCore
  name Event
  dependencies ROOT::Core HepMC3 HepMC3rootIO
  linkdef include/SimCore/Event/LinkDef.h
)

# TODO check if this is necessary
install(FILES /usr/local/lib/libHepMC3rootIO_rdict.pcm DESTINATION lib)

# include dark brem simulation
add_subdirectory(G4DarkBreM)

# Library of our interactions with G4
setup_library(module SimCore
              name G4User
              dependencies Framework::Configure
                           Framework::Exception
                           Framework::Framework
                           Geant4::Interface)


# Set the LHE Reading Library
setup_library(module SimCore 
              name LHE
              dependencies Framework::Exception
                           Geant4::Interface)

# Setup the biasing operators library
setup_library(module SimCore
              name BiasOperators
              dependencies Geant4::Interface
                           Framework::Configure
                           Framework::Framework
                           G4DarkBreM)

# PN models library needs access to G4User
setup_library(module SimCore
              name PhotoNuclearModels
              dependencies SimCore::G4User)


# Primary Generators library needs G4User and LHE
setup_library(module SimCore
              name Generators
              dependencies SimCore::LHE
                           SimCore::G4User
			                     SimCore::Event
                           GENIE::GENIE
                           HepMC3)
			   
# Reweight Library
setup_library(module SimCore
              name Reweight
              dependencies Framework::Configure
                           Framework::Framework
                           GENIE::GENIE
                           HepMC3)

# Sensitive Detectors library
setup_library(module SimCore
              name SDs
              dependencies DetDescr::DetDescr
                           SimCore::Event
                           SimCore::G4User)

setup_library(module SimCore
              name Geo
              dependencies SimCore::G4User
                           DetDescr::DetDescr)

# Setup the library
setup_library(module SimCore
              dependencies ROOT::Physics
                           ROOT::MathMore
                           HepMC3
                           SimCore::G4User
                           SimCore::Geo
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
add_executable(g4-vis ${PROJECT_SOURCE_DIR}/app/g4_vis.cxx)
target_link_libraries(g4-vis PRIVATE Geant4::Interface SimCore::SimCore)
install(TARGETS g4-vis DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
