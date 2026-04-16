from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")

p.max_tries_per_event = 10000

from LDMX.Biasing import target
from LDMX.SimCore import generators as gen


det = "ldmx-ti-v15-8gev"

my_sim = target.electro_nuclear(det, gen.single_8gev_e_upstream_tagger())
my_sim.description = "Ti Target EN Simulation"

p.sequence = [my_sim]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys


# this sample takes about 20 times more than the other CI wfs
# so we divide with 20 to get the same ballpark in time
p.max_events = int(os.environ["LDMX_NUM_EVENTS"]) // 20
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist.root"
p.output_files = ["events.root"]

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

# Load the TS modules
from LDMX.TrigScint.trig_scint import (
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
from LDMX.Recon import pf_reco
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


trigger = TriggerProcessor(beam_energy=8000.0, instance_name="trigger")

count = ElectronCounter(
    simulated_electron_number=1,
    instance_name="ElectronCounter",
    input_pass_name="",
)



# Load the DQM modules
from LDMX.DQM import dqm


target_dqm = [
    dqm.TrigScintSimDQM(
        instance_name="TargetSimHits",
        hit_collection="TargetSimHits",
        pad="target",
    ),
    dqm.TrigScintDigiDQM(
        instance_name="TargetDigis",
        hit_collection="TargetDigis",
        pad="target",
    ),
    dqm.TrigScintClusterDQM(
        instance_name="TargetClusters",
        cluster_collection="TargetClusters",
        pad="target",
    ),
    dqm.TrigScintDigiVerifierDQM(
        instance_name="TrigScintDigiVerifier",
        ts_simhit_coll="TargetSimHits",
        ts_digi_coll="TargetDigis",
    ),
]


# Load ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet = ecal_vetos.EcalPnetVetoProcessor()

# Load hcal veto
import LDMX.Hcal.hcal as hcal


hcal_veto = hcal.HcalVetoProcessor()

# Load preselection skimmer
from LDMX.Recon.ecal_preselection_skimmer import EcalPreselectionSkimmer


ecal_pres_skimmer = EcalPreselectionSkimmer()

ecal_pf = pf_reco.PFEcalClusterProducer()
hcal_pf = pf_reco.PFHcalClusterProducer()
track_pf = pf_reco.PFTrackProducer()
pf_comb = pf_reco.PFProducer()

p.logger.term_level = 1
# Example to show trace level logging for ecal veto (only)
# p.logger.custom(ecal_veto, level = -1)

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity
# resolution, GSF, DQM
p.sequence.extend(full_tracking_sequence.sequence)
p.sequence.extend(full_tracking_sequence.dqm_sequence)

p.sequence.extend(
    [
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
        ecal_pf,
        hcal_pf,
        track_pf,
        pf_comb,
        *target_dqm,
        dqm.PhotoNuclearDQM(),
        dqm.EcalClusterAnalyzer(),
    ]
)

p.sequence.extend(dqm.all_dqm)
