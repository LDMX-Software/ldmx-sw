import argparse
import sys
from LDMX.Framework import ldmxcfg

"""
Simulation of particles through TB prototype
"""

parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}')
parser.add_argument('--nevents',default=100,type=int)
parser.add_argument('--particle',default='neutron') # other options, mu-,e-,pi-,proton
parser.add_argument('--energy',default=2.0,type=float)
arg = parser.parse_args()

p = ldmxcfg.Process('sim')
p.max_events = arg.nevents
p.term_log_level = 0
p.log_frequency = 10

detector = 'ldmx-hcal-prototype-v2.0' # TODO: CHANGE TO FEFIX version

p.output_files = [
        arg.particle
        +f"Sim_{arg.energy:.2f}GeV_"
        + str(p.max_events)
        + f"_{detector}.root"
        ]

from LDMX.SimCore import simulator
import LDMX.Ecal.ecal_geometry # geometry required by sim

my_sim = simulator.simulator('my_sim')
my_sim.setDetector(detector)

# Get a pre-written generator
from LDMX.SimCore import generators as gen
my_gun = gen.gun(instance_name='my_gun')
my_gun.particle = arg.particle
my_gun.position = [ 0., 0., -600. ] # mm
my_gun.direction = [ 0., 0., 1] # forward in z
my_gun.energy = arg.energy # GeV
my_sim.generators = [ my_gun ]
p.sequence.append( my_sim )
# my_sim.verbosity = 1

# import chip/geometry (hardcoded) conditions
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_testbeamsim_conditions
import LDMX.Hcal.digi as hcal_digi

# add them to the sequence
p.sequence.extend(
    [
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        hcal_digi.HcalSingleEndRecProducer(
            pass_name = 'sim', coll_name = 'HcalDigis',
            rec_coll_name = 'HcalSingleEndRecHits',
        ),
        hcal_digi.HcalDoubleEndRecProducer(
            pass_name = '',
            coll_name = 'HcalSingleEndRecHits',
            rec_coll_name = 'HcalDoubleEndRecHits',
            rec_pass_name = ''
        )
    ]
)

# View configuration before actually running
p.pause()
