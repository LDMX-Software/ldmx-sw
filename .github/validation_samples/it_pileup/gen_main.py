from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxTriesPerEvent = 1000
p.maxEvents = 1000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
mySim = ecal.photo_nuclear('ldmx-det-v12',gen.single_4gev_e_upstream_tagger())
mySim.description = 'ECal PN Test Simulation'

p.sequence = [ mySim ]

import LDMX.Ecal.EcalGeometry

p.outputFiles = ['ecal_pn.root']
