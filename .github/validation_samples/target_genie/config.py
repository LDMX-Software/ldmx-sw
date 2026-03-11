from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('test')
import os

p.max_tries_per_event = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim


det = 'ldmx-det-v15-8gev'
my_sim = sim.simulator('sim')
my_sim.setDetector(det, include_scoring_planes_minimal = True)
genie = gen.genie(name='genie_G18_02a_02_11b',
                        energy = 8.0,
                        targets = [ 1000741820, 1000741830, 1000741840, 1000741860 ],
                        target_thickness = 0.3504,
                        abundances = [ 0.2650, 0.1431, 0.3064, 0.2843 ],
                        time = 0.0,
                        position = [0.,0.,0.],
                        beam_size = [ 20., 80. ],
                        direction = [0.,0.,1.],
                        tune='G18_02a_02_11b',
                        spline_file=f'{os.environ["CI_DATA"]}/target_genie/gxspl_emode_GENIE_v3_04_00.xml',
                        message_threshold_file=f'Messenger_ErrorOnly.xml')


my_sim.generators = [ genie ]

from LDMX.SimCore import genie_reweight


genie_rw = genie_reweight.GenieReweightProducer(name='genie_reweight')
genie_rw.hepmc3_coll_name = "SimHepMC3Events"
genie_rw.hepmc3_pass_name = ""
genie_rw.var_types = ["GENIE_INukeTwkDial_MFP_pi","GENIE_INukeTwkDial_MFP_N"]
genie_rw.verbosity = 0


p.sequence.append(genie_rw)

p.sequence = [ my_sim ]

##################################################################
# Below should be the same for all sim scenarios

import sys


p.max_events = int(int(os.environ['LDMX_NUM_EVENTS']) * 0.7)
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

# Load the TS modules
# Cant run this until we figure out how to have
# an upstream tagger track (GENIE starts at target)

# from LDMX.TrigScint.trig_scint import TrigScintDigiProducer
# from LDMX.TrigScint.trig_scint import TrigScintClusterProducer
# from LDMX.TrigScint.trig_scint import trig_scint_track
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
from LDMX.Ecal import ecal_trig_digi
from LDMX.Hcal import hcal_trig_digi
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor
from LDMX.Trigger import trigger_energy_sums


count = ElectronCounter(simulated_electron_number=1, instance_name='ElectronCounter', input_pass_name='')

en_trigger = [
        ecal_trig_digi.EcalTrigPrimDigiProducer(),
        hcal_trig_digi.HcalTrigPrimDigiProducer(),
        trigger_energy_sums.EcalTPSelector(),
        trigger_energy_sums.TrigEcalEnergySum(),
        trigger_energy_sums.TrigHcalEnergySum(),
        trigger_energy_sums.TrigEcalClusterProducer(),
        trigger_energy_sums.TrigElectronProducer(prop_map_name=f'{os.environ["CI_DATA"]}/target_genie/propagationMap.root'),
        trigger_energy_sums.HcalTPSelector(),
        trigger_energy_sums.HCalTrigMipReco(),
        trigger_energy_sums.ECalTrigMipReco(),

        ]

#Load PF reconstruction
from LDMX.Recon import pf_reco


pf_reco = pf_reco.pfTruthProducer()

# Load the dEdx mass estimator
from LDMX.Recon import track_dedx_mass_estimator
recoil_track_mass_estimator = track_dedx_mass_estimator.TrackDeDxMassEstimator()

# Load the DQM modules
from LDMX.DQM import dqm


# Load ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet =  ecal_vetos.EcalPnetVetoProcessor()

# Load hcal veto
import LDMX.Hcal.hcal as hcal


hcal_veto = hcal.HcalVetoProcessor()

p.logger.term_level = 10
# Example to show trace level logging for ecal veto (only)
p.logger.custom(full_tracking_sequence.dqm_recoil_ckf, level = -1)

# Add full tracking for both recoil trackers: digi, seeds, CFK, ambiguity resolution, GSF, DQM
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
        *en_trigger,
        pf_reco,
        recoil_track_mass_estimator
        ])

# Remove TS DQM
almost_all_dqm = [dqm.sample_validation_dqm + recoil_tracker_dqm + dqm.ecal_dqm + dqm.hcal_dqm + dqm.trigger_dqm + dqm.dEdx_dqm]

p.sequence.extend(*almost_all_dqm)

