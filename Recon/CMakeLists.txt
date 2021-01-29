
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Recon VERSION 2.1.0
              DESCRIPTION "Module containing recon processors that span multiple
                           systems."
              LANGUAGES CXX
)


option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

  # Search and configure ROOT
  find_package(ROOT CONFIG REQUIRED)

  register_event_object(module_path "Recon/Event" namespace "ldmx" 
                        class "CalorimeterHit" type "collection" )
  register_event_object( module_path "Recon/Event" namespace "ldmx" 
                         class "TriggerResult" )
  register_event_object( module_path "Recon/Event" namespace "ldmx" 
                         class "HgcrocDigiCollection" )
                       register_event_object( module_path "Recon/Event" 
                         namespace "ldmx" class "HgcrocTrigDigi" 
                         type "collection" )
   setup_library(module Recon name Event
                 dependencies ROOT::Core
                 register_target)
  
  return()
endif()

file(GLOB SRC_FILES CONFIGURE_DEPENDS 
  ${PROJECT_SOURCE_DIR}/src/Recon/Examples/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Recon/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Recon/Skims/[a-zA-z]*.cxx
)

setup_library(module Recon
  dependencies Framework::Framework Recon::Event Tools::Tools 
                           DetDescr::DetDescr Ecal::Ecal
              sources ${SRC_FILES}
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Recon)
