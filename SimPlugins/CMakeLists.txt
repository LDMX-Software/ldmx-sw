# declare SimPlugins module
module(
  NAME SimPlugins
  DEPENDENCIES Exception SimCore
  EXTERNAL_DEPENDENCIES Geant4
  EXTRA_LINK_LIBRARIES ${CMAKE_DL_LIBS} 
)
