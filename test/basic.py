from LDMX.Framework import ldmxcfg
# create my process object
p = ldmxcfg.Process( "test" )
# how many events to process?
import sys
p.maxEvents = 10
if len(sys.argv) > 1 :
    p.maxEvents = int(sys.argv[1])
p.run = 9001
# we want to see every event
p.logFrequency = 1
p.termLogLevel = 0
# we also only have an output file
p.outputFiles = [ "justSim_" + str(p.maxEvents) + "_events.root" ]
from LDMX.SimCore import simulator as sim
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v12' , True )
# Get a pre-written generator
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
# add your configured simulation to the sequence
mySim.description = 'Basic test Simulation'
p.sequence.append( mySim )
