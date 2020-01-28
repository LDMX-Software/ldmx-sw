# declare SimApplication module
module(
  NAME SimApplication
  DEPENDENCIES Exception Event Framework DetDescr SimCore SimPlugins Biasing
  EXTERNAL_DEPENDENCIES Geant4 ROOT Python
)
