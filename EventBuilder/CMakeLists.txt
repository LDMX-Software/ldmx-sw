cmake_minimum_required(VERSION 3.12)

# Set the project name
project(EventBuilder DESCRIPTION "for building complete events from the raw data."
             LANGUAGES CXX
)

setup_library(
  module EventBuilder
  sources src/Builder.cc
  dependencies Framework::Framework
               Packing::Packing
)

setup_python(package_name LDMX/EventBuilder)