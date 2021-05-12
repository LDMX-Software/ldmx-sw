from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('comp')
p.run = 1
p.maxEvents = 10

import sys
image_tag = sys.argv[1].replace('/','_').replace(':','_')

p.outputFiles = [ f'{image_tag}_inclusive.root' ]

from LDMX.SimCore import simulator as sim
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v12' )
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
mySim.description = 'Basic test Simulation'

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions

import LDMX.Ecal.digi as ecal_digi
import LDMX.Hcal.digi as hcal_digi

p.sequence = [ mySim,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer()
        ]
