# declare SimApplication module
module(
  NAME SimApplication
  EXECUTABLES src/ldmx_sim.cxx
  DEPENDENCIES Event Framework DetDescr SimPlugins Biasing
  EXTERNAL_DEPENDENCIES Geant4 ROOT
)
