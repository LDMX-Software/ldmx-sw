from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("pileup")

import os


p.run = 10# int(os.environ["LDMX_RUN_NUMBER"])
# slightly less than the others to test wrapping
p.max_events = 10 #int(int(os.environ["LDMX_NUM_EVENTS"]) * 0.95) // 2
p.logger.term_level = 4

from LDMX.SimCore import simulator as sim


my_sim = sim.Simulator(instance_name="my_sim")
my_sim.set_detector("ldmx-det-v15-8gev", include_scoring_planes_minimal=True)
from LDMX.SimCore import generators as gen


my_sim.generators.append(gen.single_8gev_e_upstream_tagger())
my_sim.description = "Basic test Simulation"

p.sequence = [my_sim]

import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry


p.output_files = ["pileup.root"]
