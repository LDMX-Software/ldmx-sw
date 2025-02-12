from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

from LDMX.SimCore import simulator as sim
mySim = sim.simulator( "mySim" )
det = 'ldmx-det-v14-8gev'
mySim.setDetector(det, True )
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_8gev_e_upstream_tagger() )
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Basic test Simulation'

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])

p.histogramFile = 'hist.root'
p.outputFiles = ['events.root']

# Load the full tracking sequance
from LDMX.Tracking import full_tracking_sequence

# Load the ECAL modules
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos

# Load the HCAL modules
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Hcal.digi as hcal_digi

# Load the TS modules
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

# Load the DQM modules
from LDMX.DQM import dqm

# Load electron counting and trigger
from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

# Load ecal veto and dont use tracking in it
ecalVeto = ecal_vetos.EcalVetoProcessor()
ecalVeto.recoil_from_tracking = False

# Load HCAL veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()

p.logger.termLevel = 1

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity resolution, GSF, DQM
p.sequence.extend(full_tracking_sequence.sequence)

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecalVeto,
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        hcal_veto,
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack,
        count, TriggerProcessor('trigger', 8000.),
        dqm.PhotoNuclearDQM(verbose=False),
        ])

p.sequence.extend(dqm.all_dqm)
