"""CORSIKA 8 cosmic muons through the ESA roof onto the cosmic test stand"""

import os

from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")

from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim


# injection plane just above the roof concrete, centered on the stand
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
# no EcalGeometry for the cosmic frame yet
my_sim.sensitive_detectors = [
    sd for sd in my_sim.sensitive_detectors if "EcalSD" not in sd.class_name
]

p.sequence = [my_sim]

##################################################################
# Below should be the same for all sim scenarios

p.max_events = int(os.environ["LDMX_NUM_EVENTS"])
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist.root"
p.output_files = ["events.root"]
# roof secondaries make large events, only the histograms are compared
p.skim_default_is_drop()

import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.DQM import dqm


p.sequence.extend(
    [
        dqm.SimObjects(),
        *[d for d in dqm.trig_scint_dqm if "Sim" in d.instance_name],
    ]
)

p.logger.term_level = int(os.environ.get("LDMX_LOG_LEVEL", 1))
