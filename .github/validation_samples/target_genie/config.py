from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxTriesPerEvent = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim
det = 'ldmx-det-v14-8gev'
mySim = sim.simulator('sim')
mySim.setDetector(det, True)
genie = gen.genie(name=f'genie_G18_02a_02_11b',
                        energy = 8.0,
                        targets = [ 1000741820, 1000741830, 1000741840, 1000741860 ],
                        target_thickness = 0.3504,
                        abundances = [ 0.2650, 0.1431, 0.3064, 0.2843 ],
                        time = 0.0,
                        position = [0.,0.,0.],
                        beam_size = [ 20., 80. ],
                        direction = [0.,0.,1.],
                        tune='G18_02a_02_11b',
                        spline_file=f'gxspl_emode_GENIE_v3_04_00.xml',
                        message_threshold_file=f'Messenger_ErrorOnly.xml')

mySim.generators = [ genie ]

from LDMX.SimCore import genie_reweight
genie_rw = genie_reweight.GenieReweightProducer(name='genie_reweight')
genie_rw.hepmc3CollName = "SimHepMC3Events"
genie_rw.hepmc3PassName = ""
genie_rw.var_types = ["GENIE_INukeTwkDial_MFP_pi","GENIE_INukeTwkDial_MFP_N"]
genie_rw.verbosity = 0


p.sequence.append(genie_rw)

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

p.maxEvents = int(int(os.environ['LDMX_NUM_EVENTS']) * 0.7)
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
import LDMX.Hcal.digi as hcal_digi
hcal_digi_reco = hcal_digi.HcalSimpleDigiAndRecProducer()

# Load the TS modules
# Cant run this until we figure out how to have
# an upstream tagger track (GENIE starts at target)

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

en_trigger = [
        ecal_trig_digi.EcalTrigPrimDigiProducer(),
        hcal_trig_digi.HcalTrigPrimDigiProducer(),
        trigger_energy_sums.EcalTPSelector(),
        trigger_energy_sums.TrigEcalEnergySum(),
        trigger_energy_sums.TrigHcalEnergySum(),
        trigger_energy_sums.TrigEcalClusterProducer(),
        trigger_energy_sums.TrigElectronProducer('propagationMap.root'),

        ]

#Load PF reconstruction
from LDMX.Recon import pfReco
pf_reco = pfReco.pfTruthProducer()

# Load the DQM modules
from LDMX.DQM import dqm

# Load ecal veto and use tracking in it
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()

# Load hcal veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()

p.logger.termLevel = 10
# Example to show trace level logging for ecal veto (only)
p.logger.custom(full_tracking_sequence.dqm_recoil_cfk, level = -1)

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
    full_tracking_sequence.dqm_recoil_cfk,
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
        hcal_digi.HcalDigiProducer(),
        hcal_digi_reco,
        hcal_veto,
        TriggerProcessor('trigger', 8000.),
        *en_trigger,
        pf_reco,
        *recoil_tracker_dqm
        ])

# Remove TS DQM
almost_all_dqm = [dqm.sample_validation_dqm + dqm.ecal_dqm + dqm.hcal_dqm + dqm.trigger_dqm]

p.sequence.extend(*almost_all_dqm)

