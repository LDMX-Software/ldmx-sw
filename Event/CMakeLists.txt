
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Event VERSION 2.1.0
              DESCRIPTION "Event classes used to persist LDMX simulation and physics objects." 
              LANGUAGES CXX
)

declare_event_object( class "CalorimeterHit" collection "ON" )
#declare_event_object( class "FindableTrackResult" collection "ON" )
#declare_event_object( class "HcalHit" collection "ON" )
#declare_event_object( class "HcalVetoResult" )
#declare_event_object( class "SiStripHit" collection "ON" )
#declare_event_object( class "TrigScintHit" collection "ON" )
#declare_event_object( class "TrackerVetoResult" )
#declare_event_object( class "TriggerResult" )
#declare_event_object( class "RawHit" collection "ON")

