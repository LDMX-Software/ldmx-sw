
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Event VERSION 2.1.0
              DESCRIPTION "Event classes used to persist LDMX simulation and physics objects." 
              LANGUAGES CXX
)

# Execute the command to extract the SHA1 hash of the current git tag. 
# The variable GIT_SHA1 will be set to contain the hash. 
execute_process (
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_SHA1
)

# Remove the newline character
string(REGEX REPLACE "\n$" "" GIT_SHA1 "${GIT_SHA1}")

# Copies the file 'Version.h.in', substitutes the value of GIT_SHA1 and
# writes it out to Version.h.  
configure_file(
    ${PROJECT_SOURCE_DIR}/include/Event/Version.h.in
    ${PROJECT_SOURCE_DIR}/include/Event/Version.h
)
install(FILES ${PROJECT_SOURCE_DIR}/include/Event/Version.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Event/)

declare_event_object( class "CalorimeterHit" module "Event" collection "ON" )
declare_event_object( class "ClusterAlgoResult" module "Event" )
declare_event_object( class "EcalCluster" module "Event" collection "ON" )
declare_event_object( class "EcalHit" module "Event" collection "ON" )
declare_event_object( class "EcalDigiCollection" module "Event" )
declare_event_object( class "EcalVetoResult" module "Event" )
declare_event_object( class "FindableTrackResult" module "Event" collection "ON" )
declare_event_object( class "HcalHit" module "Event" collection "ON" )
declare_event_object( class "HcalVetoResult" module "Event" )
declare_event_object( class "SimCalorimeterHit" module "Event" collection "ON" )
declare_event_object( class "SimTrackerHit" module "Event" collection "ON" )
declare_event_object( class "SiStripHit" module "Event" collection "ON" )
declare_event_object( class "SimParticle" module "Event" map "int" )
declare_event_object( class "TrigScintHit" module "Event" collection "ON" )
declare_event_object( class "TrackerVetoResult" module "Event" )
declare_event_object( class "TriggerResult" module "Event" )
declare_event_object( class "RawHit" module "Event" collection "ON")

