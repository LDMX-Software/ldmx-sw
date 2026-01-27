from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_pn')

import os
p.maxTriesPerEvent = 1000
p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS']) // 2
p.logger.termLevel = 4


from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
mySim = ecal.photo_nuclear('ldmx-det-v15-8gev',gen.single_8gev_e_upstream_tagger())
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'ECal PN Test Simulation'

p.sequence = [ mySim ]

import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry

p.outputFiles = ['ecal_pn.root']
