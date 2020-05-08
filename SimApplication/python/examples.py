
from LDMX.Framework import ldmxcfg
from LDMX.Detectors.makePath import makeDetectorPath
from LDMX.SimApplication import generators

def inclusive_single_e() :
    sim = ldmxcfg.Producer( "inclusive_single_e" , "ldmx::Simulator" )
    sim.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-v12" )
    sim.parameters[ "runNumber" ] = 1
    sim.parameters[ "description"] = "One 4GeV electron shot from far upstream."
    sim.parameters[ "generators" ] = [ generators.single_4gev_e_upstream_tagger() ]
    return sim

def lheExample( lheFile ) :
    sim = ldmxcfg.Producer( "lheSimulation" , "ldmx::Simulator" )
    sim.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-v12" )
    sim.parameters[ "runNumber" ] = 1
    sim.parameters[ "verbosity" ] = 3
    sim.parameters[ "description"] = "Example of how to use LHE generator"
    sim.parameters[ "generators" ] = [ generators.lhe( "LHEExample" , lheFile ) ]
    return sim

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

