"""Ecal-focused config script

This config script allows the user to choose the energy and angle
of the electron as it enters the front face of the ECal. After the
simulation, it runs the EcalDigiProducer, EcalRecProducer, and
EcalTrigPrimDigiProducer for studying how the Ecal handles the
input electron.

    ldmx fire ecal-resolution-study-with-trig-prim.py --help
"""

import argparse
import os

parser = argparse.ArgumentParser()

parser.add_argument('--n-events',default=10,type=int,help='number of events to simulate')
parser.add_argument('--run',default=1,help='run number (controls random number seeding)')
parser.add_argument('--out-dir',default=os.getcwd(),help='directory to put output file into')
parser.add_argument('--energy',default=4.,help='energy of incident electron in GeV')
parser.add_argument('--theta',default=0,help='angle incident electron makes with respect to z-axis in degrees')
parser.add_argument('--phi', default=0, help='azimuthal angle incident electron makes')
parser.add_argument('--angle-at-target',action='store_true', help='have the initial position shift so that the angles are from the target')

arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

if not os.path.isdir(arg.out_dir) :
    raise KeyError(f'Need to create output directory {arg.out_dir}')

p = ldmxcfg.Process( "valid" )
p.run = arg.run
p.maxEvents = arg.n_events
p.maxTriesPerEvent = 10000
file_stub = f'energy_{arg.energy}_theta_{arg.theta:02d}_phi_{arg.phi}_attarget_{arg.angle_at_target}_geometry_v14_events_{arg.n_events}_run_{arg.run}.root'
p.outputFiles = [ arg.out_dir+'/type_events_'+file_stub ]

# we want to see every event
p.logFrequency = 1000
p.termLogLevel = 0

from LDMX.SimCore import simulator
from LDMX.SimCore import generators
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_trig_digi as ecal_trig_digi

electrons = generators.gun('ecal-electrons')
electrons.particle = 'e-'
electrons.energy = arg.energy
from math import sin, cos, tan, pi
phi = arg.phi * pi/180
theta = arg.theta * pi/180
electrons.direction = [sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)]
# the front of the ECal is at z=240mm
# the last recoil tracking layer is at z=188mm
# we don't want anything else mucking up the particle before hitting the Ecal
#  so that we can easily know what the "true" value of something is, so
#  we want to keep z between ~200 and 240
z = 220. # mm - in front of ECal but skipping tracker/target material
# if we wanted the angles to be angles _from the target_, we need to 
#  shift the electron transverse (x and y) position as well so that
#  the electron avoids material /and/ looks like it came from the target
# or we could just have it be angles without moving the particle
#  this would be helpful for seeing how the angle of the particle
#  affects the resolution without having to worry about containment as much
electrons.position = [z*tan(theta)*cos(phi), z*tan(theta)*sin(phi), z] if arg.angle_at_target else [0., 0., z]

validator = simulator.simulator('plain')
validator.setDetector(f'ldmx-det-v14', False)
validator.description = 'Electrons straight into ECal for ECal geometry testing'
validator.generators = [electrons]

# turn off all PN interactions so we have a "clean" EM shower
from LDMX.SimCore import photonuclear_models as pn
validator.photonuclear_model = pn.NoPhotoNuclearModel()

digi = ecal_digi.EcalDigiProducer()
prec_rec = ecal_digi.EcalRecProducer()
trig_digi = ecal_trig_digi.EcalTrigPrimDigiProducer()

p.sequence = [ validator, digi, prec_rec, trig_digi ]
