# declare SimApplication module
module(
  NAME SimApplication
  EXECUTABLES src/ldmx_sim.cxx
  DEPENDENCIES Exception Event Framework DetDescr SimCore SimPlugins Biasing
  EXTERNAL_DEPENDENCIES Geant4 ROOT Python
)
