#!/usr/bin/python

################################################################################################
# @file examples.py
# Example Simulators using various generators
#
# @author Tom Eichlersmith, University of Minnesota
################################################################################################

from LDMX.Framework import ldmxcfg
from LDMX.Detectors.makePath import makeDetectorPath
from LDMX.SimApplication import generators

################################################################################################
# @func basicOneElectron
# @return a simulation with one 4GeV electron shot from far upstream
################################################################################################
def basicOneElectron() :
    sim = ldmxcfg.Producer( "basicOneElectron" , "ldmx::Simulator" )
    sim.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-v12" )
    sim.parameters[ "runNumber" ] = 1
    sim.parameters[ "description"] = "One 4GeV electron shot from far upstream."
    sim.parameters[ "generators" ] = [ generators.farUpstreamSingle4GeVElectron() ]
    return sim

################################################################################################
# @func lheExample
# @param lheFile path to LHE file to use with generator
# @return a simulator using the LHE generator
################################################################################################
def lheExample( lheFile ) :
    sim = ldmxcfg.Producer( "lheSimulation" , "ldmx::Simulator" )
    sim.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-v12" )
    sim.parameters[ "runNumber" ] = 1
    sim.parameters[ "verbosity" ] = 3
    sim.parameters[ "description"] = "Example of how to use LHE generator"
    sim.parameters[ "generators" ] = [ generators.lhe( "LHEExample" , lheFile ) ]
    return sim

################################################################################################
# @func gpsExample
# @return a simulator using the General Particle Source
################################################################################################
def gpsExample( ) :
    sim = ldmxcfg.Producer( "gpsExample" , "ldmx::Simulator" )
    sim.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-v12" )
    sim.parameters[ "runNumber" ] = 1
    sim.parameters[ "verbosity" ] = 3
    sim.parameters[ "description"] = "Example of how to use GPS generator"
    gpsGen = generators.gps( "GPSExample" )
    gpsGen.parameters[ "initCommands" ] = [
        "/gps/particle e-",
        "/gps/pos/type Plane",
        "/gps/pos/shape Square",
        "/gps/pos/centre 0 0 0 mm",
        "/gps/pos/halfx 40 mm",
        "/gps/pos/halfy 80 mm",
        "/gps/ang/type cos",
        "/gps/ene/type Lin",
        "/gps/ene/min 3 GeV",
        "/gps/ene/max 4 GeV",
        "/gps/ene/gradient 1",
        "/gps/ene/intercept 1"
        ]
    sim.parameters[ "generators" ] = [ gpsGen ]
    return sim

################################################################################################
# @func multiExample
# @return a simulator using the MPG generator
################################################################################################
def multiExample( ) :
    sim = ldmxcfg.Producer( "mpgExample" , "ldmx::Simulator" )
    sim.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-v12" )
    sim.parameters[ "runNumber" ] = 1
    sim.parameters[ "verbosity" ] = 3
    sim.parameters[ "description"] = "Example of how to use MPG generator"
    mpgGen = generators.multi( "MPGExample" )
    mpgGen.parameters[ "vertex" ] = [ 0., 0., 0. ] #mm
    mpgGen.parameters[ "nParticles" ] = 2
    mpgGen.parameters[ "pdgID" ] = 11
    mpgGen.parameters[ "enablePoisson" ] = True
    mpgGen.parameters[ "momentum" ] = [ 0., 0., 4000. ] #MeV
    sim.parameters[ "generators" ] = [ mpgGen ]
    return sim

