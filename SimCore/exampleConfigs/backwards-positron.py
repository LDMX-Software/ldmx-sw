"""fire a positron with the input energy backwards from the target

This is helpful for the situation where the beam spot needs to be
determined. Instead of doing complicated math to figure out how much
an electron of a given energy curves, let's just use a positron and Geant4
to do the math for us!

Note
----
This only functions well if the detector components upstream of the target
are not included in the simulation. (If they are, then the positron interacts
with them easily spoiling the measurement.) This can be done by commenting
out their inclusion in the detector.gdml with `<!-- ... -->`.
"""

from LDMX.Framework import ldmxcfg
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--n-events', type=int, default=10, help='number of events to simulate')
parser.add_argument('--beam', type=float, required=True, help='beam energy in GeV')

args = parser.parse_args()

p = ldmxcfg.Process("beam")
p.maxEvents = args.n_events
p.termLogLevel = 1
p.run = 1
p.outputFiles = [ f'backwards_positron_beam_{args.beam}.root' ]

from LDMX.SimCore import generators
from LDMX.SimCore import simulator
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry

mySim = simulator.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v14' , True )
mySim.generators = [ generators.single_backwards_positron(args.beam) ]
mySim.description = 'Basic test Simulation'
p.sequence = [ mySim ]
