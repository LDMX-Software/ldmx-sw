from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

from LDMX.SimCore import simulator as sim
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v14-8gev', True )
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_8gev_e_upstream_tagger() )
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Basic test Simulation'

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

#p.run = int(os.environ['LDMX_RUN_NUMBER'])
#p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])

p.run = int(sys.argv[1])
p.maxEvents = int(sys.argv[2])
p.maxTriesPerEvent = int(sys.argv[3])

p.histogramFile = 'hist_8gev.root'
p.outputFiles = ['events_8gev.root']

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
ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]
for d in ts_digis :
    d.randomSeed = 1

from LDMX.DQM import dqm

from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecal_vetos.EcalVetoProcessor(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack,
        count, TriggerProcessor('trigger'),
        dqm.PhotoNuclearDQM(verbose=False),
        ] + dqm.all_dqm)
