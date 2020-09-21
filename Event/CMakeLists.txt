
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Event VERSION 2.1.0
              DESCRIPTION "Event classes used to persist LDMX simulation and physics objects." 
              LANGUAGES CXX
)

register_event_object( module_path "Event" namespace "ldmx" class "CalorimeterHit"      type "collection" )
register_event_object( module_path "Event" namespace "ldmx" class "FindableTrackResult" type "collection" )
register_event_object( module_path "Event" namespace "ldmx" class "HcalHit"             type "collection" )
register_event_object( module_path "Event" namespace "ldmx" class "HcalVetoResult"                        )
register_event_object( module_path "Event" namespace "ldmx" class "TrigScintHit"        type "collection" )
register_event_object( module_path "Event" namespace "ldmx" class "TrackerVetoResult"                     )
register_event_object( module_path "Event" namespace "ldmx" class "TriggerResult"                         )

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

setup_library(name Event 
              dependencies ROOT::Core
                           "${registered_targets}"
              register_target)

setup_python(install_path ${PYTHON_INSTALL_PREFIX}/Event)

