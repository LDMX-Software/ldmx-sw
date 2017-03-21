# declare SimPlugins module
module(
  NAME SimPlugins
  EXTRA_LINK_LIBRARIES ${Geant4_LIBRARIES}
)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})
