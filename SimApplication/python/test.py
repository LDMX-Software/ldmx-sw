#!/usr/bin/python

################################################################################################
# @file test.py
# Simulators that test various aspects of the simulation
#
# @author Tom Eichlersmith, University of Minnesota
################################################################################################

from LDMX.Framework import ldmxcfg
from LDMX.Detectors.makePath import makeDetectorPath
from LDMX.SimApplication import generators

################################################################################################
# @func geometry
# @return a simulation with one 4GeV electron shot from far upstream with the Geant4
#   overlap testing for the geometry turned on
################################################################################################
def geometry() :
    sim = ldmxcfg.Producer( "testGeometry" , "ldmx::Simulator" )
    sim.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
    sim.parameters[ "runNumber" ] = 1
    sim.parameters[ "verbosity" ] = 3
    sim.parameters[ "description"] = "One 4GeV electron shot from far upstream."
    sim.parameters[ "generators" ] = [ generators.farUpstreamSingle4GeVElectron() ]
    sim.parameters[ "postInitCommands" ] = [ "/geometry/test/run" ]
    return sim

