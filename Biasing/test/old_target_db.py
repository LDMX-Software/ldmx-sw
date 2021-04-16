from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('old_signal')

from LDMX.SimCore import simulator
sim = simulator.simulator('old_signal_sim')
sim.description = 'Place dark brem events inside target volume.'
sim.setDetector( 'ldmx-det-v12' , True )

ap_mass = 1000. #MeV
from LDMX.SimCore import makePath
db_event_lib_path = makePath.makeLHEPath(ap_mass)
print(db_event_lib_path)

import glob
possible_lhes = glob.glob( db_event_lib_path+'/*4.0*.lhe' )

if len(possible_lhes) == 1 :
    the_lhe = possible_lhes[0].strip()
else :
    raise Exception("Not exactly one LHE file simulated for 4GeV Beam and input mass")

p.maxEvents = 10 # assume LHE has at least 10 events in it

from LDMX.SimCore import generators
sim.dark_brem.activate(ap_mass)
sim.generators = [ generators.lhe('dark_brem', the_lhe ) ] 
sim.beamSpotSmear = [ 20., 80., 0.3504 ] #mm

p.outputFiles = [ '/tmp/old_signal.root' ]
