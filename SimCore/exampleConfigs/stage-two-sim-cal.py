from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("simcal")
# see stage-one-sim-no-cal.py to generate the file
p.input_files = ["stage-one-no-cal.root"]
p.output_files = ["stage-two-cal.root"]
p.log_frequency = 1
p.logger.term_level = 0
p.max_events = 5

import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.SimCore import simulator

sim = simulator.Simulator(instance_name="scoring-plane-sim")
sim.set_detector("ldmx-det-v15-8gev")

from LDMX.SimCore.generators import FromScoringPlane

sim.generators = [FromScoringPlane.hcal()]
p.sequence = [sim]
