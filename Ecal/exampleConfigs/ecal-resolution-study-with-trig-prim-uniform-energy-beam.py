"""Ecal-focused config script

This config script allows the user to choose the energy and angle
of the electron as it enters the front face of the ECal. After the
simulation, it runs the EcalDigiProducer, EcalRecProducer, and
EcalTrigPrimDigiProducer for studying how the Ecal handles the
input electron.

    ldmx fire ecal-resolution-study-with-trig-prim-uniform-energy-beam.py --help
"""

import argparse
import os

parser = argparse.ArgumentParser()

parser.add_argument('--n-events',default=10,type=int,help='number of events to simulate')
parser.add_argument('--run',default=3,help='run number (controls random number seeding)')
parser.add_argument('--out-dir',default=os.getcwd(),help='directory to put output file into')
parser.add_argument('--energy-min',default=1.,help='min energy of incident electron in GeV')
parser.add_argument('--energy-max',default=4.,help='max energy of incident electron in GeV')

arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

if not os.path.isdir(arg.out_dir) :
    raise KeyError(f'Need to create output directory {arg.out_dir}')

p = ldmxcfg.Process( "valid" )
p.run = arg.run
p.maxEvents = arg.n_events
p.maxTriesPerEvent = 10000
file_stub = '_'.join([
    'type', 'events',
    'emin', str(arg.energy_min),
    'emax', str(arg.energy_max),
    'geometry', 'v14',
    'events', str(arg.n_events),
    'run', str(arg.run)
])
p.outputFiles = [ arg.out_dir+'/'+file_stub+'.root' ]

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

electrons = generators.gps(
    'ecal-electrons',
    [
        '/gps/particle e-',
        '/gps/pos/type Point', #beamSpotSmear will smear for us
        '/gps/pos/centre 0 0 220. mm', # start in from of ECal
        '/gps/direction 0 0 1', # directly into ECal
        '/gps/ene/type Lin',
        f'/gps/ene/min {arg.energy_min} GeV',
        f'/gps/ene/max {arg.energy_max} GeV',
        '/gps/ene/gradient 0', # linear distribution flat so its uniform
        '/gps/ene/intercept 1',
        '/gps/pos/shape Square',
        '/gps/number 1'
    ]
)

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
