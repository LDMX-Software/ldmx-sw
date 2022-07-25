import argparse,sys
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
p.maxEvents = arg.nevents
p.termLogLevel = 0
p.logFrequency = 10

detector = 'ldmx-hcal-prototype-v2.0' # TODO: CHANGE TO FEFIX version

p.outputFiles = [
        arg.particle
        +"Sim_%.2fGeV_"%arg.energy
        + str(p.maxEvents) 
        + "_%s.root"%detector
        ]

from LDMX.SimCore import simulator
import LDMX.Ecal.EcalGeometry # geometry required by sim

mySim = simulator.simulator('mySim')
mySim.setDetector(detector)

# Get a pre-written generator
from LDMX.SimCore import generators as gen
myGun = gen.gun('myGun')
myGun.particle = arg.particle
myGun.position = [ 0., 0., 0 ] # mm 
myGun.direction = [ 0., 0., 1] # forward in z
myGun.energy = arg.energy # GeV
mySim.generators = [ myGun ]
p.sequence.append( mySim )
# mySim.verbosity = 1

# import chip/geometry (hardcoded) conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
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
    ]
)

# View configuration before actually running
p.pause()
