from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('ecal_pn')

import os


p.max_tries_per_event = 1000
p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.max_events = int(os.environ['LDMX_NUM_EVENTS']) // 2
p.logger.term_level = 4


from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen


my_sim = ecal.photo_nuclear('ldmx-det-v15-8gev',gen.single_8gev_e_upstream_tagger())
my_sim.description = 'ECal PN Test Simulation'

p.sequence = [ my_sim ]

import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry


p.output_files = ['ecal_pn.root']
