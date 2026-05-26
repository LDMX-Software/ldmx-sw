from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")

p.max_tries_per_event = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen


det = "ldmx-det-v15-8gev"

my_sim = ecal.photo_nuclear(det, gen.single_8gev_e_upstream_tagger())
my_sim.description = "ECal PN Test Simulation"

p.sequence = [my_sim]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys


p.max_events = int(os.environ["LDMX_NUM_EVENTS"])
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist2.root"
p.output_files = ["events.root"]

# Load the full tracking sequence
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

from LDMX.Tracking import full_tracking_sequence
from LDMX.Tracking.tracking import TruthSeedProcessor, TruthTrackProcessor

# Configure TruthSeedProcessor
truth_seeder = TruthSeedProcessor("TruthSeedProcessor")
truth_seeder.scoring_hits_coll_name = "TargetScoringPlaneHits"
truth_seeder.sp_pass_name = ""
truth_seeder.sim_particles_coll_name = "SimParticles"
truth_seeder.sim_particles_passname = ""
truth_seeder.tagger_sim_hits_coll_name = "TaggerSimHits"
truth_seeder.recoil_sim_hits_coll_name = "RecoilSimHits"
truth_seeder.input_pass_name = ""
truth_seeder.beam_electrons_collection = "BeamElectrons"
truth_seeder.tagger_seeds_collection = "TaggerTruthSeeds"
truth_seeder.recoil_seeds_collection = "RecoilTruthSeeds"
truth_seeder.n_min_hits_tagger = 11
truth_seeder.p_cut = 0.
truth_seeder.p_cut_ecal = -1.
truth_seeder.particle_hypothesis = 11
truth_seeder.skip_tagger = False
truth_seeder.skip_recoil = False
truth_seeder.recoil_sp = True
truth_seeder.beamOrigin = [-883.0, -21.745876, 0.0]

track_constructor = TruthTrackProcessor("TruthTrackProcessor")
track_constructor.tagger_seeds_collection = "TaggerTruthSeeds"
track_constructor.recoil_seeds_collection = "RecoilTruthSeeds"
track_constructor.input_pass_name = ""
track_constructor.tagger_tracks_collection = "TaggerTruthTracks"
track_constructor.recoil_tracks_collection = "RecoilTruthTracks"
track_constructor.n_min_hits_tagger = 11
track_constructor.n_min_hits_recoil = 7
track_constructor.p_cut = 0.
track_constructor.pz_cut = -9999.0
track_constructor.particle_hypothesis = 11
track_constructor.skip_tagger = False
track_constructor.skip_recoil = False
track_constructor.seedSmearing = False
track_constructor.sim_particles_coll_name = "SimParticles"
track_constructor.sim_particles_passname = ""
track_constructor.ecal_sp_coll_name = "EcalScoringPlaneHits"
track_constructor.sp_pass_name = ""

# Overwrite the default truth processors in the sequence
full_tracking_sequence.sequence[2] = truth_seeder
full_tracking_sequence.sequence[3] = track_constructor


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

# Load electron counting and trigger
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


count = ElectronCounter(
    simulated_electron_number=1,
    instance_name="ElectronCounter",
    input_pass_name="",
)

# Load the DQM modules
from LDMX.DQM import dqm


# Define ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet = ecal_vetos.EcalPnetVetoProcessor()

# Load hcal veto
import LDMX.Hcal.hcal as hcal


hcal_veto = hcal.HcalVetoProcessor()

# Load visibles veto and turn off all
# cuts except the visibles BDT
visibles_veto = dqm.VisiblesCutflow()
visibles_veto.all_cuts = False

# Load preselection skimmer
from LDMX.Recon.ecal_preselection_skimmer import EcalPreselectionSkimmer


ecal_pres_skimmer = EcalPreselectionSkimmer()


p.logger.term_level = int(os.environ["LDMX_LOG_LEVEL"])
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
        count,
        TriggerProcessor(beam_energy=8000.0, instance_name="trigger"),
        dqm.PhotoNuclearDQM(),
        dqm.EcalClusterAnalyzer(),
        visibles_veto,
        dqm.VisiblesFeatureProducer(),
    ]
)

p.sequence.extend(dqm.all_dqm)
