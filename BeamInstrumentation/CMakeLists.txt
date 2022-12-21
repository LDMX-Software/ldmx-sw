cmake_minimum_required(VERSION 3.12)

project(BeamInstrumentation VERSION 0.0.1
	DESCRIPTION "Module for testbeam instrumentation."
	LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

option(BUILD_EVENT_ONLY "Build the event library." ON)
if(BUILD_EVENT_ONLY)

  register_event_object( module_path "BeamInstrumentation/Event" namespace "beaminstrumentation" class "FiberTracker" type "collection" )
  register_event_object( module_path "BeamInstrumentation/Event" namespace "beaminstrumentation" class "WhiteRabbitResult" type "collection" )
  
	# Generate the files needed to build the event classes.
	setup_library(module BeamInstrumentation name Event
		dependencies ROOT::Core
		Hcal::Event
		TrigScint::Event
		Recon::Event
		BeamInstrumentation::Event
	register_target)

	return()

endif()

setup_library(module BeamInstrumentation
  dependencies Framework::Framework Recon::Event DetDescr::DetDescr Tools::Tools SimCore::Event BeamInstrumentation::Event
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/BeamInstrumentation)
