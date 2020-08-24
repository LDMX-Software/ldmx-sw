
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

declare_event_object( class "CalorimeterHit" collection "ON" )
declare_event_object( class "ClusterAlgoResult" )
declare_event_object( class "EcalCluster" collection "ON" )
declare_event_object( class "EcalHit" collection "ON" )
declare_event_object( class "EcalDigiCollection" )
declare_event_object( class "EcalVetoResult" )
declare_event_object( class "FindableTrackResult" collection "ON" )
declare_event_object( class "HcalHit" collection "ON" )
declare_event_object( class "HcalVetoResult" )
declare_event_object( class "SimCalorimeterHit" collection "ON" )
declare_event_object( class "SimTrackerHit" collection "ON" )
declare_event_object( class "SiStripHit" collection "ON" )
declare_event_object( class "SimParticle" map "int" )
declare_event_object( class "TrigScintHit" collection "ON" )
declare_event_object( class "TrackerVetoResult" )
declare_event_object( class "TriggerResult" )
declare_event_object( class "RawHit" collection "ON")

