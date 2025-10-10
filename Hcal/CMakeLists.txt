cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Hcal DESCRIPTION "Module for all things HCal."
             LANGUAGES CXX
)

setup_library(
  module Hcal
  name Event
  dependencies ROOT::Core Recon::Event
  linkdef include/Hcal/Event/LinkDef.h
)

setup_library(module Hcal
              dependencies ROOT::Physics 
              Framework::Framework Recon::Event Tools::Tools DetDescr::DetDescr 
              Tracking::Event Ecal::Event Hcal::Event
)

setup_python(package_name LDMX/Hcal)

setup_test(dependencies Hcal::Hcal)

setup_data(module Hcal)
