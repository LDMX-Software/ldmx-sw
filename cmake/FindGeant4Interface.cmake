#
# FindGeant4Interface.cmake
#
# Find an installation of Geant4 and wrap it with a CMake target.
# Highly tuned to the installation of Geant4 that is built into the
# ldmx/dev image.
# Just builds a target Geant4::Interface around the already-defined
# Geant4 CMake configuration installed with the build.
#

if (TARGET Geant4::Interface)
  return()
endif()

include(CMakeFindDependencyMacro)
find_dependency(Geant4 REQUIRED gdml ui_all vis_all)

# Geant4_DEFINITIONS already include -D, this leads to the error
# <command-line>:0:1: error: macro names must be identifiers if not removed.
set(G4_DEF_TEMP "")
foreach(def ${Geant4_DEFINITIONS})
  string(REPLACE "-D" "" def ${def})
  list(APPEND G4_DEF_TEMP ${def})
endforeach()
set(LDMX_Geant4_DEFINITIONS ${G4_DEF_TEMP})
unset(G4_DEF_TEMP)

# Create the Geant4 target
add_library(Geant4::Interface INTERFACE IMPORTED GLOBAL)

# Set the target properties
set_target_properties(
  Geant4::Interface
  PROPERTIES INTERFACE_LINK_LIBRARIES "${Geant4_LIBRARIES}"
             INTERFACE_COMPILE_OPTIONS "${Geant4_Flags}"
             INTERFACE_COMPILE_DEFINITIONS "${LDMX_Geant4_DEFINITIONS}"
             INTERFACE_INCLUDE_DIRECTORIES "${Geant4_INCLUDE_DIRS}")

message(STATUS "Found Geant4 version ${Geant4_VERSION}")
