from LDMX.Framework import ldmxcfg
# create my process object
p = ldmxcfg.Process( "test" )
# how many events to process?
import sys
p.maxEvents = 10
if len(sys.argv) > 1 :
    p.maxEvents = int(sys.argv[1])
# we want to see every event
p.logFrequency = 1
p.termLogLevel = 0
# Set a run number
p.run = 9001
# we also only have an output file
p.outputFiles = [ "/dev/null" ]
from LDMX.SimCore import simulator as sim
#import LDMX.Ecal.EcalGeometry
#import LDMX.Hcal.HcalGeometry
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v14' , True )
# only use first SD which is a TrigScint SD and so it doesn't need the Ecal/Hcal geometry compiled
mySim.sensitive_detectors = [
    mySim.sensitive_detectors[0]
]
# Get a pre-written generator
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
# add your configured simulation to the sequence
mySim.description = 'Basic test Simulation'
p.sequence.append( mySim )
p.libraries.append('/home/tom/code/ldmx/ldmx-sw/build/clang-lto/install/lib/libSimCore_SDs.so')
