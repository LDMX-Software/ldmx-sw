"""Example configuration for reading HepMC events
This example demonstrates how to read events from a HepMC file
and simulate them through the LDMX detector.
"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("hepmc_sim")
p.max_events = 10000
p.run = 1
p.output_files = ["hepmc_simulation.root"]
p.histogram_file = "hist_hepmc_test.root"

from LDMX.SimCore import generators
from LDMX.SimCore import simulator
import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry

# Create HepMC generator
hepmc_gen = generators.HepMC(
    instance_name="hepmc_generator",
    file_path= "/home/yan_xu/project-2/work_area/project3_hadronic_decay/1p5_pythia8_events.hepmc",
    vertex=[0.0, 0.0, 0.0],
    beam_spot_smear=[20.0, 80.0, 0.0]
)


# Configure simulator
my_sim = simulator.Simulator("my_sim")
my_sim.set_detector("ldmx-det-v15-8gev", include_scoring_planes_minimal=True)
my_sim.generators = [hepmc_gen]
my_sim.description = "HepMC Event Simulation"

p.logger.term_level = 10
p.logger.custom(my_sim, level = -1)
p.logger.custom(hepmc_gen, level = -1)
p.logger.custom("HepMCReader", level = -1)



p.sequence = [my_sim]

from LDMX.DQM import dqm
p.sequence.append(dqm.SampleValidation())