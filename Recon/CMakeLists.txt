cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Recon DESCRIPTION "Module containing recon processors that span multiple
                           systems."
              LANGUAGES CXX
)

setup_library(
  module Recon
  name Event
  dependencies ROOT::Core
  linkdef include/Recon/Event/LinkDef.h
)

file(GLOB SRC_FILES CONFIGURE_DEPENDS 
  ${PROJECT_SOURCE_DIR}/src/Recon/Examples/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Recon/[a-zA-z]*.cxx
  ${PROJECT_SOURCE_DIR}/src/Recon/Skims/[a-zA-z]*.cxx
)

setup_library(module Recon
  dependencies Framework::Framework Recon::Event Tools::Tools 
               DetDescr::DetDescr Ecal::Event
               Packing::Utility TrigScint::Event Tracking::Event
  sources ${SRC_FILES}
)

setup_python(package_name LDMX/Recon)
