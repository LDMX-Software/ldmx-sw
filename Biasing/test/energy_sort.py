from LDMX.Framework import ldmxcfg
# create my process object
p = ldmxcfg.Process( "test" )
# how many events to process?
p.maxEvents = 1
# we want to see every event
p.logFrequency = 1
p.termLogLevel = 0
# we also only have an output file
p.outputFiles = [ "/tmp/energy_sort.root" ]
from LDMX.SimCore import simulator as sim
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
from LDMX.Biasing import util
mySim.actions = [ util.PartialEnergySorter( 3000. ) ]
p.sequence.append( mySim )
p.pause()
