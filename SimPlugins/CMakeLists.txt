# find Geant4
find_package(Geant4 REQUIRED ui_all vis_all)

# declare SimPlugins module
module(
  NAME SimPlugins
  EXTRA_LINK_LIBRARIES ${Geant4_LIBRARIES}
  DEPENDENCIES SimCore
)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})
