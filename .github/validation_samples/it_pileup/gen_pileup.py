from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('pileup')

import os
p.run = int(os.environ['LDMX_RUN_NUMBER'])
# slightly less than the others to test wrapping
p.maxEvents = int(int(os.environ['LDMX_NUM_EVENTS'])*0.95) // 2
p.logger.termLevel = 4

from LDMX.SimCore import simulator as sim
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v15-8gev' )
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_8gev_e_upstream_tagger() )
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Basic test Simulation'

p.sequence = [ mySim ]

import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry

p.outputFiles = ['pileup.root']
