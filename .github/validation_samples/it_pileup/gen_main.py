from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

import os
p.maxTriesPerEvent = 1000
p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])


from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
mySim = ecal.photo_nuclear('ldmx-det-v12',gen.single_4gev_e_upstream_tagger())
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'ECal PN Test Simulation'

p.sequence = [ mySim ]

import LDMX.Ecal.EcalGeometry

p.outputFiles = ['ecal_pn.root']
