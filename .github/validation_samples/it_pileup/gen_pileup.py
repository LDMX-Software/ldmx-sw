from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('pileup')

import os


p.run = int(os.environ['LDMX_RUN_NUMBER'])
# slightly less than the others to test wrapping
p.max_events = int(int(os.environ['LDMX_NUM_EVENTS'])*0.95) // 2
p.logger.term_level = 4

from LDMX.SimCore import simulator as sim


my_sim = sim.simulator( "my_sim" )
my_sim.setDetector( 'ldmx-det-v15-8gev' )
from LDMX.SimCore import generators as gen


my_sim.generators.append( gen.single_8gev_e_upstream_tagger() )
my_sim.beamSpotSmear = [20.,80.,0.]
my_sim.description = 'Basic test Simulation'

p.sequence = [ my_sim ]

import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry


p.output_files = ['pileup.root']
