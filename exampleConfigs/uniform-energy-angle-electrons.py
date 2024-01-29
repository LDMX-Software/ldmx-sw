import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--max-energy',help='maximum energy [GeV] to sample from', default=4.0, type=float)
parser.add_argument('--min-energy',help='minimum energy [GeV] to sample from', default=0.0, type=float)
parser.add_argument('--angle',help='maximum polar angle [degrees] to sample from', default=60.0, type=float)
parser.add_argument('--n-events',help='number of events to simulate',default=10,type=int)
args = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('uniele')
p.maxEvents = args.n_events
p.run = 1

p.outputFiles = ['uniform-electrons.root']

import LDMX.Hcal.HcalGeometry
import LDMX.Ecal.EcalGeometry

from LDMX.SimCore import simulator
from LDMX.SimCore import generators

sim = simulator.simulator("uniform-electrons")
sim.setDetector('ldmx-det-v14-8gev', True)
sim.description = "Electrons with uniformly sampled energy and angle shot from target"
sim.beamSpotSmear = [20., 80., 0.]
# GPS generator
sim.generators = [
    generators.gps('uniform-electrons', [
        # electrons
        '/gps/particle e-',
        # position distribution: all from the same point, simulator smears beam spot
        '/gps/pos/type Point', # beamSpotSmear will smear for us
        '/gps/pos/centre 0 0 0 mm', # shoot from center of target
        # angular distribution, isotropic with maximum polar angle relative to z-axis
        '/gps/ang/type iso', # isotropic angular distribution
        '/gps/ang/mintheta 0 deg', # minimum polar angle
        f'/gps/ang/maxtheta {args.angle} deg', # maximum polar angle
        '/gps/ang/minphi 0 deg', # minimum azimuthal angle
        '/gps/ang/maxphi 360 deg', # maximum azimuthal angle
        # energy distribution, uniform between the two configured limits
        '/gps/ene/type Lin', # linear distribution (will set slope to zero)
        f'/gps/ene/min {args.min_energy} GeV',
        f'/gps/ene/max {args.max_energy} GeV',
        '/gps/ene/gradient 0', # make linear distribution flat
        '/gps/ene/intercept 1',
        # one particle per event
        '/gps/number 1',
    ])
]

p.sequence = [
    sim
]
