from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('test')

p.max_tries_per_event = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim


det = 'ldmx-det-v15-8gev'
my_sim = sim.simulator( instance_name="my_sim" )
my_sim.setDetector(det, include_scoring_planes_minimal = True )
my_sim.description = 'Deep ECal Gun Simulation'

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
gps_cmds = ['/gps/particle e-',
            *ene_ang_pos_cmds_ele,
            '/gps/source/add 1', '/gps/particle gamma',
            *ene_ang_pos_cmds_gamma,
            '/gps/source/multiplevertex True']

my_sim.generators = [gen.gps(instance_name='electron_photon', init_commands=gps_cmds)]

p.sequence = [ my_sim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys


p.max_events = int(os.environ['LDMX_NUM_EVENTS'])
p.run = int(os.environ['LDMX_RUN_NUMBER'])

p.histogram_file = 'hist.root'
p.output_files = ['events.root']

# Load the full tracking sequance
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_clusters as ecal_cluster

# Load the ECAL modules
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi_and_reco

# Load the HCAL modules
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions
from LDMX.Tracking import full_tracking_sequence


hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()


# Load electron counting and trigger
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


count = ElectronCounter(simulated_electron_number=1, instance_name='ElectronCounter', input_pass_name='')

trigger = TriggerProcessor(beam_energy=8000., instance_name='trigger')

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

p.logger.term_level = 1
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

p.skim_default_is_drop()
p.skim_consider(trigger.instance_name)

almost_all_dqm = [
        dqm.sample_validation_dqm
        + dqm.ecal_dqm
        + dqm.hcal_dqm
        + dqm.trigger_dqm
        ]

p.sequence.extend(*almost_all_dqm)
