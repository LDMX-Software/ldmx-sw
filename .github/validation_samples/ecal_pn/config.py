from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxEvents = 10

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
mySim = ecal.photo_nuclear('ldmx-det-v12',gen.single_4gev_e_upstream_tagger())
mySim.description = 'ECal PN Test Simulation'

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

p.run = 1

os.makedirs('hist',exist_ok=True)
os.makedirs('events',exist_ok=True)
sample_id = os.path.basename(sys.argv[0]).replace('.py','')
p.histogramFile = f'hist/{sample_id}.root'
p.outputFiles = [f'events/{sample_id}.root']

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi

from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack
tsDigisUp   = TrigScintDigiProducer.up()
tsDigisTag  = TrigScintDigiProducer.tagger()
tsDigisDown = TrigScintDigiProducer.down()
tsDigisUp.randomSeed = 1
tsDigisTag.randomSeed = 1
tsDigisDown.randomSeed = 1
clTag=TrigScintClusterProducer.tagger()
clUp=TrigScintClusterProducer.up()
clDown=TrigScintClusterProducer.down()

from LDMX.DQM import dqm

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecal_vetos.EcalVetoProcessor(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        tsDigisUp, tsDigisTag, tsDigisDown,
        clTag, clUp, clDown,
        trigScintTrack,
        ] + dqm.trigScint_dqm + dqm.ecal_dqm + dqm.recoil_dqm)
