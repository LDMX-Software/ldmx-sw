from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxTriesPerEvent = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim
det = 'ldmx-det-v15-8gev'
mySim = sim.simulator( "mySim" )
mySim.setDetector(det, True )
mySim.description = 'Deep ECal Gun Simulation'

ene_ang_pos_cmds_ele = [
        '/gps/ene/type Lin',
        '/gps/ene/min 1 GeV',
        '/gps/ene/max 3 GeV',
        '/gps/ene/gradient 0.',
        '/gps/ene/intercept 1.',
        '/gps/ang/type cos',
        '/gps/pos/type Plane',
        '/gps/pos/shape Square',
        '/gps/pos/centre 0 0 600 mm',
        '/gps/pos/halfx 100 mm',
        '/gps/pos/halfy 100 mm'
        ]

ene_ang_pos_cmds_gamma = [
        '/gps/ene/type Lin',
        '/gps/ene/min 3 GeV',
        '/gps/ene/max 5 GeV',
        '/gps/ene/gradient 0.',
        '/gps/ene/intercept 1.',
        '/gps/ang/type cos',
        '/gps/pos/type Plane',
        '/gps/pos/shape Square',
        '/gps/pos/centre 0 0 600 mm',
        '/gps/pos/halfx 100 mm',
        '/gps/pos/halfy 100 mm'
        ]

# One electron and one photon both with the above initial kinematics
gps_cmds = (['/gps/particle e-'] + 
            ene_ang_pos_cmds_ele + 
            ['/gps/source/add 1', '/gps/particle gamma'] + 
            ene_ang_pos_cmds_gamma + 
            ['/gps/source/multiplevertex True'])

mySim.generators = [gen.gps('electron_photon', gps_cmds)]

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])
p.run = int(os.environ['LDMX_RUN_NUMBER'])

p.histogramFile = f'hist.root'
p.outputFiles = [f'events.root']

# Load the full tracking sequance
from LDMX.Tracking import full_tracking_sequence

# Load the ECAL modules
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Ecal.ecalClusters as ecal_cluster

# Load the HCAL modules
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Hcal.digi as hcal_digi_and_reco
hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()


# Load electron counting and trigger
from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

trigger = TriggerProcessor('trigger', 8000.)

# Load the DQM modules
from LDMX.DQM import dqm

# Define ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_veto.recoil_from_tracking = False

ecal_mip = ecal_vetos.EcalMipProcessor()

ecal_veto_pnet =  ecal_vetos.EcalPnetVetoProcessor()
ecal_veto_pnet.recoil_from_tracking = False

# Load hcal veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()

p.logger.termLevel = 1
# Example to show trace level logging for trigger (only)
p.logger.custom(trigger, level = -1)

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecal_cluster.EcalClusterProducer(),
        ecal_veto,
        ecal_mip,
        ecal_veto_pnet,
        hcal_digi,
        hcal_reco,
        hcal_veto,
        trigger,
        dqm.EcalClusterAnalyzer()
        ])

p.skimDefaultIsDrop()
p.skimConsider(trigger.instanceName)

almost_all_dqm = [dqm.sample_validation_dqm + dqm.ecal_dqm + dqm.hcal_dqm + dqm.trigger_dqm]

p.sequence.extend(*almost_all_dqm)
