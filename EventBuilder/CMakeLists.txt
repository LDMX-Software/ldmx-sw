cmake_minimum_required(VERSION 3.12)
# Set the project name
project(EventBuilder
  DESCRIPTION "for building complete events from the raw data."
  LANGUAGES CXX
)

# Set up the main EventBuilder library
setup_library(
  module EventBuilder
  dependencies Framework::Framework
               Packing::Packing
)

# Configure RPATH for the library
# Set empty RPATH to ensure the library has no embedded runtime path dependencies
set_target_properties(EventBuilder PROPERTIES
  BUILD_RPATH ""
  INSTALL_RPATH ""
  INSTALL_RPATH_USE_LINK_PATH OFF
)

setup_python(package_name LDMX/EventBuilder)
