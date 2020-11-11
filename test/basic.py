from LDMX.Framework import ldmxcfg
# create my process object
p = ldmxcfg.Process( "test" )
# how many events to process?
p.maxEvents = 10
# we want to see every event
p.logFrequency = 1
p.termLogLevel = 0
# we also only have an output file
p.outputFiles = [ "justSim_" + str(p.maxEvents) + "_events.root" ]
from LDMX.SimCore import simulator as sim
from LDMX.Ecal import EcalGeometry
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v12' )
# Set a run number
mySim.runNumber = 9001
# Get a pre-written generator
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
# add your configured simulation to the sequence
mySim.description = 'Basic test Simulation'
mySim.randomSeeds = [ 1 , 2 ]

p.sequence.append( mySim )
