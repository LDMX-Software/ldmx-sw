# find Geant4
find_package(Geant4 REQUIRED ui_all vis_all)

# find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO)

# declare SimApplication module
module(
  NAME SimApplication
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  EXECUTABLES src/ldmx_sim.cxx
  EXTRA_LINK_LIBRARIES ${Geant4_LIBRARIES} ${ROOT_LIBRARIES}
  DEPENDENCIES Event Framework DetDescr SimCore SimPlugins Biasing
)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})
