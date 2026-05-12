import os

from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("eat")

p.max_tries_per_event = 10000

from LDMX.Biasing import eat
from LDMX.SimCore import generators


det = "ldmx-det-v15-8gev"
sim = eat.dark_brem(
    ap_mass=10.0,
    db_event_lib=(
        f"{os.environ['CI_DATA']}/signal/"
        "v5.2.0_electron_tungsten_MaxE_8.0_MinE_4.0"
        "_RelEStep_0.1_UndecayedAP_mA_0.01_run_1.csv"
    ),
    detector=det,
    generator=generators.single_8gev_e_upstream_tagger(),
)

p.sequence = [sim]

##################################################################
# Below should be the same for all sim scenarios

p.max_events = int(os.environ["LDMX_NUM_EVENTS"])
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist.root"
p.output_files = ["events.root"]

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.vetos as ecal_vetos


ecal_veto = ecal_vetos.EcalVetoProcessor(
    recoil_from_tracking = False
)

import LDMX.Hcal.digi as hcal_digi
import LDMX.Hcal.hcal as hcal
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions
from LDMX.DQM import dqm
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor
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

counter = ElectronCounter(
    simulated_electron_number=1,
    instance_name="ElectronCounter",
    input_pass_name="",
)

trigger = TriggerProcessor(beam_energy=8000.0, instance_name="trigger")
trigger.thresholds = [3160.0]

p.logger.term_level = 2
p.logger.custom(dqm.DarkBremInteraction(), level = 0)

p.sequence.extend(
    [
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_veto,
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        hcal.HcalVetoProcessor(),
        *ts_digis,
        *ts_clusters,
        trig_scint_track,
        counter,
        trigger,
        dqm.EcalSPElectronKinematics(),
        dqm.DarkBremInteraction(),
        dqm.HcalVetoResults(),
    ]
)
