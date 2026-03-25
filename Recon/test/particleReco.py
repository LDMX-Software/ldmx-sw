from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("PF")
import sys


p.max_events = 10
if len(sys.argv) > 1:
    p.max_events = int(sys.argv[1])

# we want to see every event
p.log_frequency = 1 if p.max_events <= 10 else 100
p.term_log_level = 1

# Set a run number
p.run = 9001

# we also only have an output file
p.output_files = ["pfReco_" + str(p.max_events) + "_events.root"]

import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.SimCore import simulator as sim


my_sim = sim.Simulator(instance_name="my_sim")
my_sim.set_detector("ldmx-det-v14", include_scoring_planes_minimal=True)
sim.beamSpotSmear = [20.0, 80.0, 0.0]

# Get a pre-written generator
from particleSources import cocktail_commands

from LDMX.SimCore import generators as gen


my_sim.generators.append(
    gen.Gps(instance_name="my_gps", init_commands=cocktail_commands)
)
# add your configured simulation to the sequence
p.sequence.append(my_sim)

# reco stuff

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions
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
for d in ts_digis:
    d.randomSeed = 1

from LDMX.DQM import dqm
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


count = ElectronCounter(
    simulated_electron_number=1,
    instance_name="ElectronCounter",
    input_pass_name="",
)

from LDMX.Ecal import ecal_trig_digi
from LDMX.Hcal import hcal_trig_digi


p.sequence.extend(
    [
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
    ]
)

if True:  # False:
    p.set_compression(2, level=9)  # LZMA
    from LDMX.Recon import pf_reco

    ecal_pf = pf_reco.PFEcalClusterProducer()
    hcal_pf = pf_reco.PFHcalClusterProducer()
    track_pf = pf_reco.PFTrackProducer()
    truth_pf = pf_reco.PFTruthProducer()

    # configure clustering options
    ecal_pf.do_single_cluster = False
    ecal_pf.log_energy_weight = True

    hcal_pf.do_single_cluster = False
    hcal_pf.cluster_hit_dist = 200.0  # mm
    hcal_pf.log_energy_weight = True

    ecal_pf_simple = pf_reco.PFEcalClusterProducer()
    ecal_pf_simple.cluster_coll_name += "Simple"
    ecal_pf_simple.do_single_cluster = True
    hcal_pf_simple = pf_reco.PFHcalClusterProducer()
    hcal_pf_simple.cluster_coll_name += "Simple"
    hcal_pf_simple.do_single_cluster = True

    p.sequence.extend(
        [
            ecal_pf,
            hcal_pf,
            track_pf,
            pf_reco.PFProducer(),
            truth_pf,
            # ecal_pf_simple, hcal_pf_simple
        ]
    )
