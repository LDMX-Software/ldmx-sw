from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('test')

p.max_tries_per_event = 10000

from LDMX.Biasing import target
from LDMX.SimCore import generators as gen


det = 'ldmx-lyso-r4-v15-8gev'
my_sim = target.photo_nuclear(det, gen.single_8gev_e_upstream_tagger())
my_sim.beamSpotSmear = [20.,80.,0.]
my_sim.description = 'LYSO Target PN Simulation'

p.sequence = [ my_sim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys


# this sample takes about 5 times more than the other CI wfs
# so we divide with 5 to get the same ballpark in time
p.max_events = int(os.environ['LDMX_NUM_EVENTS']) // 5
p.run = int(os.environ['LDMX_RUN_NUMBER'])

p.histogram_file = 'hist.root'
p.output_files = ['events.root']

# Load the full tracking sequance
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.ecalClusters as ecal_cluster

# Load the ECAL modules
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi_and_reco
import LDMX.Hcal.hcal_hardcoded_conditions

# Load the HCAL modules
import LDMX.Hcal.HcalGeometry
from LDMX.Tracking import full_tracking_sequence


hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()

# Load the TS modules
from LDMX.TrigScint.trigScint import (
        TrigScintClusterProducer,
        TrigScintDigiProducer,
        trig_scint_track,
)


ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]

ts_clusters = [
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        ]

target_digis = TrigScintDigiProducer.target()
target_clusters = TrigScintClusterProducer.target()

# Load electron counting and trigger
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


trigger = TriggerProcessor('trigger', 8000.)

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

# Load the DQM modules
from LDMX.DQM import dqm


target_dqm = [
    dqm.TrigScintSimDQM('TargetSimHits','TargetSimHits','target'),
    dqm.TrigScintDigiDQM('TargetDigis','TargetDigis','target'),
    dqm.TrigScintClusterDQM('TargetClusters','TargetClusters','target'),
    dqm.TrigScintDigiVerifierDQM('TrigScintDigiVerifier','TargetSimHits','TargetDigis'),
    ]


# Load ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet =  ecal_vetos.EcalPnetVetoProcessor()

# Load hcal veto
import LDMX.Hcal.hcal as hcal


hcal_veto = hcal.HcalVetoProcessor()

# Load preselection skimmer
from LDMX.Recon.ecalPreselectionSkimmer import EcalPreselectionSkimmer


ecal_pres_skimmer = EcalPreselectionSkimmer()

p.logger.term_level = 1
# Example to show trace level logging for ecal veto (only)
# p.logger.custom(ecal_veto, level = -1)

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity resolution, GSF, DQM
p.sequence.extend(full_tracking_sequence.sequence)
p.sequence.extend(full_tracking_sequence.dqm_sequence)

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_pres_skimmer,
        ecal_cluster.EcalClusterProducer(),
        ecal_veto,
        ecal_mip,
        ecal_veto_pnet,
        hcal_digi,
        hcal_reco,
        hcal_veto,
        *ts_digis,
        *ts_clusters,
        trig_scint_track,
        target_digis,
        target_clusters,
        count,
        trigger,
        *target_dqm,
        dqm.PhotoNuclearDQM(),
        dqm.EcalClusterAnalyzer()
        ])

p.sequence.extend(dqm.all_dqm)

