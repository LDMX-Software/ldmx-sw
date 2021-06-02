from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

# slightly less than the others to test wrapping
p.maxEvents = 10

from LDMX.SimCore import simulator as sim
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v12' )
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
mySim.description = 'Basic test Simulation'

p.sequence = [ mySim ]
p.run = 1

import LDMX.Ecal.EcalGeometry

p.outputFiles = ['pileup.root']
