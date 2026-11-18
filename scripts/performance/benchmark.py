""" Run ecal_pn sequence to gather performance metrics in a specified mode. """

from LDMX.Framework import ldmxcfg

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore.user_actions import PhotonuclearTracker

import os

# Configurable sim-only / recon-only / full modes
mode = os.environ.get("LDMX_MODE", "full").strip().lower()

# Check if valid mode was entered
if mode not in ["sim", "recon", "full"]:
    raise SystemExit("Error, enter 'sim', 'recon', or 'full' for mode")

p = ldmxcfg.Process(mode)
p.max_tries_per_event = 10000

# Pinned detector version
det = "ldmx-det-v16-8gev"

p.max_tries_per_event = 10000

# Takes random seed and event count
run = int(os.environ["LDMX_RUN_NUMBER"])
max_events = int(os.environ["LDMX_NUM_EVENTS"])

# Fixed log level
log_level = 100
 
p.run = run
p.max_events = max_events
p.logger.term_level = log_level
p.log_performance= True

out_dir = f"scripts/performance/root/run_{run}"
os.makedirs(out_dir, exist_ok=True)
p.histogram_file = f"{out_dir}/hist_{mode}_{max_events}.root"
p.output_files = [f"{out_dir}/events_{mode}_{max_events}.root"]

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
ecal_tracking = ecal_vetos.EcalTrackFinderProcessor()

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


# Configure simulation
if mode in ["sim", "full"]:
    my_sim = ecal.photo_nuclear(det, gen.single_8gev_e_upstream_tagger())
    my_sim.description = "ECal PN Test Simulation"

    # Enable detailed photonuclear interaction tracking
    my_sim.actions.append(PhotonuclearTracker())

    p.sequence.append(my_sim)

if mode in ["recon", "full"]:
    # For recon, use input files from simulation
    if mode == "recon":
        p.input_files = [f"{out_dir}/events_sim_{max_events}.root"]

    # Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity
    # resolution, GSF, DQM
    trk = full_tracking_sequence.full_tracking_sequence(detector=det)
    p.sequence.extend(trk.sequence)
    p.sequence.extend(trk.dqm_sequence)

    p.sequence.extend(
        [
            ecal_digi.EcalDigiProducer(),
            ecal_digi.EcalRecProducer(),
            ecal_tracking,
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


