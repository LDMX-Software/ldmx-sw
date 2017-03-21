# declare Biasing module
module(
  NAME Biasing
  EXTRA_LINK_LIBRARIES ${Geant4_LIBRARIES}
  DEPENDENCIES SimPlugins 
)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})
