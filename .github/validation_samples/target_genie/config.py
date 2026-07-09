from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")
import os

from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim
from LDMX.SimCore.bias_operators import ElectroNuclear


det = "ldmx-det-v15-8gev"
# det = "ldmx-ti-v15-8gev"
my_sim = sim.Simulator(instance_name="sim")
my_sim.set_detector(det, include_scoring_planes_minimal=True)

# Beam electron as primary — tracked through tagger with natural energy loss
my_sim.generators = [gen.single_8gev_e_upstream_tagger()]

# Enable GENIE as a physics process.
# The electron is tracked normally; when Geant4 selects the process to fire,
# GENIE generates the interaction at the electron's actual energy.
my_sim.genie_nuclear.enable = True
# Targets and abundances are auto-discovered from the target region geometry.
# Discovery runs once and only builds GENIE drivers for the elements actually
# in this volume (matching the biased volume below), instead of every material
# the electron traverses upstream.
my_sim.genie_nuclear.discover_volume = "target_region"
my_sim.genie_nuclear.tune = "G18_02a_02_11b"
# Splines are stored one file per tune (gxspl_emode_<TUNE>.xml). Derive the
# filename from the tune so the two can never disagree — a mismatch makes GENIE
# silently recompute all cross sections on the fly and the job appears to hang.
# See .github/validation_samples/target_genie/README.md.
my_sim.genie_nuclear.spline_file = (
    f"{os.environ['CI_DATA']}/target_genie/gxspl_emode_{my_sim.genie_nuclear.tune}.xml"
)
my_sim.genie_nuclear.message_threshold_file = "Messenger_ErrorOnly.xml"

# Bias EN cross-section in target region so interactions occur at usable rate
my_sim.biasing_operators = [ElectroNuclear(volume="target_region", factor=1e6)]

from LDMX.SimCore import genie_reweight


genie_rw = genie_reweight.GenieReweightProducer(instance_name="genie_reweight")
genie_rw.hepmc3_coll_name = "SimHepMC3Events"
genie_rw.hepmc3_pass_name = ""
genie_rw.var_types = ["GENIE_INukeTwkDial_MFP_pi", "GENIE_INukeTwkDial_MFP_N"]
genie_rw.message_threshold_file = "Messenger_ErrorOnly.xml"


p.sequence = [my_sim, genie_rw]

##################################################################
# Below should be the same for all sim scenarios

import sys


p.max_events = int(int(os.environ["LDMX_NUM_EVENTS"]) * 0.7)
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

# Load the TS modules — now enabled since we have an upstream beam electron
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
from LDMX.Ecal import ecal_trig_digi
from LDMX.Hcal import hcal_trig_digi
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor
from LDMX.Trigger import trigger_energy_sums


count = ElectronCounter(
    simulated_electron_number=1,
    instance_name="ElectronCounter",
    input_pass_name="",
)

en_trigger = [
    ecal_trig_digi.EcalTrigPrimDigiProducer(),
    hcal_trig_digi.HcalTrigPrimDigiProducer(),
    trigger_energy_sums.EcalTPSelector(),
    trigger_energy_sums.TrigEcalEnergySum(),
    trigger_energy_sums.TrigHcalEnergySum(),
    trigger_energy_sums.TrigEcalClusterProducer(),
    trigger_energy_sums.TrigElectronProducer(
        prop_map_name=(f"{os.environ['CI_DATA']}/target_genie/propagationMap.root")
    ),
    trigger_energy_sums.HcalTPSelector(),
    trigger_energy_sums.HCalTrigMipReco(),
    trigger_energy_sums.ECalTrigMipReco(),
]

# Load PF reconstruction
from LDMX.Recon import pf_reco


pf_reco = pf_reco.PFTruthProducer()

# Load the dEdx mass estimator
from LDMX.Recon import track_dedx_mass_estimator


recoil_track_mass_estimator = track_dedx_mass_estimator.TrackDeDxMassEstimator()

# Load the DQM modules
from LDMX.DQM import dqm


# Load ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet = ecal_vetos.EcalPnetVetoProcessor()

# Load hcal veto
import LDMX.Hcal.hcal as hcal


hcal_veto = hcal.HcalVetoProcessor()

p.logger.term_level = 10
# Example to show trace level logging for ecal veto (only)
# p.logger.custom(full_tracking_sequence.dqm_recoil_ckf, level=-1)
p.logger.custom("GenieElectroNuclearProcess", level=-1)


# Add full tracking for both tagger and recoil trackers:
# digi, seeds, CKF, ambiguity resolution, GSF, DQM
tagger_tracking = [
    full_tracking_sequence.digi_tagger,
    full_tracking_sequence.seeder_tagger,
    full_tracking_sequence.tracking_tagger,
    full_tracking_sequence.greedy_solver_tagger,
    full_tracking_sequence.gsf_tagger,
]

recoil_tracking = [
    full_tracking_sequence.digi_recoil,
    full_tracking_sequence.truth_tracking,
    full_tracking_sequence.seeder_recoil,
    full_tracking_sequence.tracking_recoil,
    full_tracking_sequence.greedy_solver_recoil,
    full_tracking_sequence.gsf_recoil,
]

tagger_tracker_dqm = [
    full_tracking_sequence.dqm_tagger_ckf,
]

recoil_tracker_dqm = [
    full_tracking_sequence.dqm_recoil_ckf,
    #     full_tracking_sequence.dqm_recoil_gas,
    #     full_tracking_sequence.dqm_recoil_gsf
]

p.sequence.extend(
    [
        *tagger_tracking,
        *recoil_tracking,
        *ts_digis,
        *ts_clusters,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_cluster.EcalClusterProducer(),
        ecal_veto,
        ecal_mip,
        ecal_veto_pnet,
        hcal_digi,
        hcal_reco,
        hcal_veto,
        TriggerProcessor(beam_energy=8000.0, instance_name="trigger"),
        *en_trigger,
        pf_reco,
        recoil_track_mass_estimator,
    ]
)

almost_all_dqm = [
    dqm.sample_validation_dqm
    + tagger_tracker_dqm
    + recoil_tracker_dqm
    + dqm.ecal_dqm
    + dqm.hcal_dqm
    + dqm.trigger_dqm
    + dqm.dedx_dqm
    + dqm.trig_scint_dqm
]

p.sequence.extend(*almost_all_dqm)
p.sequence.append(dqm.ElectroNuclearDQM())
