from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxTriesPerEvent = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim
# Load LHE file containing WAB events
wab_gen = gen.lhe("WAB Generator", "8GeV_WABFF2_10K.lhe") 
# Place them in the middle of the target
wab_gen.vertex = [0.0, 0.0, 0.0]

det = 'ldmx-det-v15-8gev'
mySim = sim.simulator('sim')
mySim.setDetector(det, include_scoring_planes_minimal = True)
mySim.generators.append(wab_gen) 

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

p.maxEvents = int(int(os.environ['LDMX_NUM_EVENTS']) * 0.99)
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

# Load the TS modules
# Cant run this until we figure out how to have
# an upstream tagger track (LHE info starts at target)

# from LDMX.TrigScint.trigScint import TrigScintDigiProducer
# from LDMX.TrigScint.trigScint import TrigScintClusterProducer
# from LDMX.TrigScint.trigScint import trigScintTrack
# ts_digis = [
#         TrigScintDigiProducer.pad1(),
#         TrigScintDigiProducer.pad2(),
#         TrigScintDigiProducer.pad3(),
#         ]

# ts_clusters = [
#         TrigScintClusterProducer.pad1(),
#         TrigScintClusterProducer.pad2(),
#         TrigScintClusterProducer.pad3(),
#         ]

# Load electron counting and trigger
from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor
from LDMX.Hcal import hcal_trig_digi
from LDMX.Ecal import ecal_trig_digi
from LDMX.Trigger import trigger_energy_sums

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

# Load the DQM modules
from LDMX.DQM import dqm

# Load ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet =  ecal_vetos.EcalPnetVetoProcessor()

# Load hcal veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()
hcal_clusters = hcal.HcalClusterProducer()
hcal_wab = hcal.HcalWABVetoProcessor()

p.logger.termLevel = 1
# Example to show trace level logging for recoil CKF  (only)
#p.logger.custom(full_tracking_sequence.dqm_recoil_ckf, level = -1)

# Add full tracking for both recoil trackers: digi, seeds, CKF, ambiguity resolution, GSF, DQM
recoil_tracking = [
    full_tracking_sequence.digi_recoil,
    full_tracking_sequence.truth_tracking,
    full_tracking_sequence.seeder_recoil,
    full_tracking_sequence.tracking_recoil,
    full_tracking_sequence.greedy_solver_recoil,
    full_tracking_sequence.GSF_recoil
]

recoil_tracker_dqm = [
    full_tracking_sequence.dqm_recoil_ckf,
#     full_tracking_sequence.dqm_recoil_gas,
#     full_tracking_sequence.dqm_recoil_gsf
]

p.sequence.extend([
        *recoil_tracking,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecal_cluster.EcalClusterProducer(),
        ecal_veto,
        ecal_mip,
        ecal_veto_pnet,
        hcal_digi,
        hcal_reco,
        hcal_veto,
        TriggerProcessor('trigger', 8000.),
        #hcal_clusters,
        #hcal_wab,
        *recoil_tracker_dqm,
        ])

# Remove TS DQM
almost_all_dqm = [dqm.sample_validation_dqm + dqm.ecal_dqm + dqm.hcal_dqm + dqm.trigger_dqm]

p.sequence.extend(*almost_all_dqm)

