#!/usr/bin/python

import os
import sys

from LDMX.Framework import ldmxcfg


# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
sim_pass_name = "ecal_pn"
pileup_file_pass_name = "pileup"
this_pass_name = "overlay"
p = ldmxcfg.Process(this_pass_name)

det = "ldmx-det-v15-8gev"
p.run = 10 # int(os.environ["LDMX_RUN_NUMBER"])
p.max_events = 10 # int(os.environ["LDMX_NUM_EVENTS"]) // 2


# Load the full tracking sequance
from LDMX.Recon.overlay import OverlayProducer


overlay = OverlayProducer(
    overlay_filename="pileup.root",
    sim_passname=sim_pass_name,
    overlay_passname=pileup_file_pass_name,
)
overlay.poisson_mu = 3.
p.sequence = [overlay]

# ECal geometry nonsense
import LDMX.Ecal.ecal_clusters as ecal_cluster
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.hcal_hardcoded_conditions
from LDMX.Ecal import digi as ecal_digi_reco
from LDMX.Ecal import ecal_geometry
from LDMX.Ecal import vetos as ecal_vetos

# Hcal hardwired/geometry stuff
from LDMX.Hcal import hcal_geometry


# this is hardwired into the code to be appended to the sim hits collections
overlay_str = "Overlay"

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
for digi in ts_digis:
    digi.input_collection += overlay_str
    digi.sim_particles_coll_name += overlay_str

ts_clusters = [
    TrigScintClusterProducer.pad1(),
    TrigScintClusterProducer.pad2(),
    TrigScintClusterProducer.pad3(),
]
for clu in ts_clusters:
    clu.input_pass_name = this_pass_name

trig_scint_track.input_pass_name = this_pass_name


# Load the ECAL modules
ecal_digi = ecal_digi_reco.EcalDigiProducer("ecal_digis")
ecal_reco = ecal_digi_reco.EcalRecProducer("ecal_recon")
ecal_veto = ecal_vetos.EcalVetoProcessor("ecal_veto")
ecal_mip = ecal_vetos.EcalMipProcessor("ecal_mip")

# The newly produced, overlayed simhits
ecal_digi.input_coll_name += overlay_str
ecal_digi.input_pass_name = this_pass_name

# Use the digis produced above
ecal_reco.digi_pass_name = this_pass_name
# SimHits are used to find noise
ecal_reco.sim_hit_coll_name += overlay_str
ecal_reco.sim_hit_pass_name = this_pass_name

ecal_veto.recoil_from_tracking = False
ecal_veto.rec_pass_name = this_pass_name
ecal_veto.ecal_sp_coll_name += overlay_str
ecal_veto.target_sp_coll_name += overlay_str
ecal_veto.track_pass_name = this_pass_name
ecal_veto.sim_particles_coll_name += overlay_str

ecal_veto_pnet = ecal_vetos.EcalPnetVetoProcessor()
ecal_veto_pnet.ecal_rec_hits_passname = this_pass_name
ecal_veto_pnet.ecal_sp_coll_name += overlay_str
ecal_veto_pnet.track_pass_name = this_pass_name

ecal_mip.ecal_pass_name = this_pass_name
ecal_mip.mip_pass_name = this_pass_name

# Load the HCAL modules
import LDMX.Hcal.digi as hcal_digi_and_reco


hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()
# The newly produced, overlayed simhits
hcal_digi.input_coll_name += overlay_str
hcal_digi.input_pass_name = this_pass_name
# Use the digis produced above
hcal_reco.input_pass_name = this_pass_name
hcal_reco.sim_hit_coll_name += overlay_str

# Load ElectronCounter and Trigger
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


count = ElectronCounter(
    simulated_electron_number=2,
    instance_name="ElectronCounter",
    input_pass_name=this_pass_name,
)

# Load HCAL veto
from LDMX.Hcal.hcal import HcalVetoProcessor


hcal_veto = HcalVetoProcessor()
hcal_veto.input_hit_pass_name = this_pass_name
hcal_veto.track_pass_name = this_pass_name

# Load and configure  particle flow sequence.
# Here we use PF "tracking" and CLUE Ecal clustering
from LDMX.Recon import pf_reco


track_pf = pf_reco.PFTrackProducer()
track_pf.input_track_coll_name += overlay_str  # "EcalScoringPlaneHitsOverlay"
track_pf.input_pass_name = this_pass_name
track_pf.do_electron_tracking = True
# reference info
truth_pf = pf_reco.PFTruthProducer()
truth_pf.target_sp_coll_name += overlay_str
truth_pf.ecal_sp_coll_name += overlay_str
truth_pf.sim_particles_coll_name += overlay_str

# CLUE
import LDMX.Ecal.ecal_clusters as cl

cluster = cl.EcalClusterProducer()
cluster.seed_threshold = 350.0
cluster.dc = 0.3
cluster.nbr_of_layers = 1
cluster.reclustering = True
cluster.rec_hit_pass_name = this_pass_name  # run on process+pileup

# DBScan-based ECAL and HCAL cluster producers
ecal_pf = pf_reco.PFEcalClusterProducer()
ecal_pf.hit_pass_name = this_pass_name
hcal_pf = pf_reco.PFHcalClusterProducer()
hcal_pf.hit_pass_name = this_pass_name

# particle flow:
pf_comb = pf_reco.PFProducer()
pf_comb.input_ecal_coll_name = cluster.cluster_coll_name  # use CLUE
pf_comb.input_ecal_passname = this_pass_name
pf_comb.input_hcal_passname = this_pass_name
pf_comb.input_tracks_passname = this_pass_name

# trigger recasting existing CLUE to caloclusters
pf_comb.use_existing_ecal_clusters = True

# Load pileup finder
from LDMX.Recon import pileup_finder


pu_finder = pileup_finder.PileupFinder()
pu_finder.rec_hit_pass_name = this_pass_name
# needs recast caloclusters, not (CLUE) ecalclusters
pu_finder.cluster_coll_name = pf_comb.input_ecal_coll_name + "Cast"
pu_finder.pf_cand_coll_name = pf_comb.output_coll_name
pu_finder.pf_cand_pass_name = this_pass_name
pu_finder.min_momentum = 3000.0

# Load the DQM modules
from LDMX.DQM import dqm


trig_scint_sim_dqm = [
    dqm.TrigScintSimDQM(
        instance_name="TrigScintSimPad1",
        hit_collection="TriggerPad1SimHits",
        pad="pad1",
    ),
    dqm.TrigScintSimDQM(
        instance_name="TrigScintSimPad2",
        hit_collection="TriggerPad2SimHits",
        pad="pad2",
    ),
    dqm.TrigScintSimDQM(
        instance_name="TrigScintSimPad3",
        hit_collection="TriggerPad3SimHits",
        pad="pad3",
    ),
]

for ts_sim_dqm in trig_scint_sim_dqm:
    ts_sim_dqm.hit_collection += overlay_str

trig_scint_dqm = [
    dqm.TrigScintDigiDQM(
        instance_name="TrigScintDigiPad1",
        hit_collection="trigScintDigisPad1",
        pad="pad1",
    ),
    dqm.TrigScintDigiDQM(
        instance_name="TrigScintDigiPad2",
        hit_collection="trigScintDigisPad2",
        pad="pad2",
    ),
    dqm.TrigScintDigiDQM(
        instance_name="TrigScintDigiPad3",
        hit_collection="trigScintDigisPad3",
        pad="pad3",
    ),
    dqm.TrigScintClusterDQM(
        instance_name="TrigScintClusterPad1",
        cluster_collection="TriggerPad1Clusters",
        pad="pad1",
    ),
    dqm.TrigScintClusterDQM(
        instance_name="TrigScintClusterPad2",
        cluster_collection="TriggerPad2Clusters",
        pad="pad2",
    ),
    dqm.TrigScintClusterDQM(
        instance_name="TrigScintClusterPad3",
        cluster_collection="TriggerPad3Clusters",
        pad="pad3",
    ),
    dqm.TrigScintTrackDQM(
        instance_name="TrigScintTracks",
        track_collection="TriggerPadTracks",
    ),
]

for ts_dqm in trig_scint_dqm:
    ts_dqm.pass_name = this_pass_name

# EcalDigiVerify
ecal_digi_verify = dqm.EcalDigiVerify()
ecal_digi_verify.ecal_sim_hit_coll += overlay_str
ecal_digi_verify.ecal_rec_hit_pass = this_pass_name

# EcalShowerFeatures
ecal_shower_features = dqm.EcalShowerFeatures()
ecal_shower_features.ecal_veto_pass = this_pass_name

# EcalMipTrackingFeatures
ecal_mip_tracking_features = dqm.EcalMipTrackingFeatures()
ecal_mip_tracking_features.ecal_veto_pass = this_pass_name
ecal_mip_tracking_features.ecal_mip_pass = this_pass_name

# EcalVetoResults
ecal_veto_results = dqm.EcalVetoResults()
ecal_veto_results.ecal_veto_pass = this_pass_name

# HCAL DQM
hcal_dqm = [
    dqm.HCalDQM(pe_veto_threshold=8.0, section=0),
    dqm.HCalDQM(pe_veto_threshold=8.0, section=1),
    dqm.HCalDQM(pe_veto_threshold=8.0, section=2),
    dqm.HCalDQM(pe_veto_threshold=8.0, section=3),
    dqm.HCalDQM(pe_veto_threshold=8.0, section=4),
    dqm.HcalInefficiencyAnalyzer(),
]

for hdqm in hcal_dqm:
    hdqm.rec_pass_name = this_pass_name
    hdqm.sim_coll_name += overlay_str

# Trigger DQM
trigger_dqm = dqm.Trigger()
trigger_dqm.trigger_pass = this_pass_name

dqm_with_overlay = (
    trig_scint_sim_dqm
    + trig_scint_dqm
    + [
        trigger_dqm,
        ecal_digi_verify,
        ecal_shower_features,
        ecal_mip_tracking_features,
        ecal_veto_results,
    ]
    + hcal_dqm
)

p.logger.term_level = 1

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK,
# ambiguity resolution, GSF, DQM
from LDMX.Tracking import full_tracking_sequence


# append "Overlay" to sim collection names in tracking sequence
full_tracking_sequence.set_overlay(this_pass_name)
p.sequence.extend(full_tracking_sequence.sequence)
p.sequence.extend(full_tracking_sequence.dqm_sequence)

p.sequence.extend(
    [
        ecal_digi,
        ecal_reco,
        ecal_veto,
        ecal_mip,
        ecal_veto_pnet,
        hcal_digi,
        hcal_reco,
        hcal_veto,
        *ts_digis,
        *ts_clusters,
        trig_scint_track,
        count,
        TriggerProcessor(beam_energy=8000.0, instance_name="trigger"),
        dqm.PhotoNuclearDQM(),
    ]
)

p.sequence.extend(dqm_with_overlay)

# Add PFlow + pileup finding sequence
p.sequence.extend(
    [
        cluster,
        dqm.EcalClusterAnalyzer(),
        track_pf,
        truth_pf,
        ecal_pf,
        hcal_pf,
        pf_comb,
        pu_finder,
    ]
)

p.input_files = ["ecal_pn.root"]
p.output_files = ["events.root"]
p.histogram_file = "hist.root"

