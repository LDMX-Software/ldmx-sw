# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(TrigScint VERSION 2.1.0
                  DESCRIPTION "Module for all things trigger scintillator."
                  LANGUAGES CXX
)

setup_library(
  module TrigScint
  name Event
  dependencies ROOT::Core Hcal::Event Recon::Event
  linkdef include/TrigScint/Event/LinkDef.h
)

setup_library(
  module TrigScint
  name Firmware
  dependencies Tools::Tools
)

setup_library(
  module TrigScint
  dependencies Framework::Framework Recon::Event Hcal::Event DetDescr::DetDescr
               Tools::Tools SimCore::Event TrigScint::Firmware TrigScint::Event
)

setup_python(package_name LDMX/TrigScint)

