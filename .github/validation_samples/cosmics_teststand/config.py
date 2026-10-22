"""CORSIKA 8 cosmic muons through the ESA roof onto the cosmic test stand"""

import os

from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")

from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim


# injection plane above the roof concrete, muons aimed at the ECal
cosmics = gen.HepMC(
    instance_name="cosmics",
    file_path=f"{os.environ['CI_DATA']}/cosmics_teststand/corsika8_muons.hepmc3.root",
    vertex=[5000.0, -110.0, 16000.0],
    beam_spot_smear=[],
)

det = "ldmx-esa-cosmic26-v1"
my_sim = sim.Simulator(instance_name="sim")
my_sim.set_detector(det)
my_sim.generators = [cosmics]
my_sim.time_shift_primaries = False

p.sequence = [my_sim]

##################################################################
# Below should be the same for all sim scenarios

# muons are cheap, run twice the default, the input holds 30k
p.max_events = min(2 * int(os.environ["LDMX_NUM_EVENTS"]), 20000)
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist.root"
p.output_files = ["events.root"]

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.hcal_geometry
from LDMX.DQM import dqm


p.sequence.extend(
    [
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        dqm.EcalDigiVerify(),
        dqm.SimObjects(),
        *[d for d in dqm.trig_scint_dqm if "Sim" in d.instance_name],
    ]
)

p.logger.term_level = int(os.environ.get("LDMX_LOG_LEVEL", 1))
