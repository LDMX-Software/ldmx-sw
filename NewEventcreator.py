#Creates simulation sample .root file with TS info

from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

from LDMX.SimCore import simulator as sim
from LDMX.SimCore import generators as gen

mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v14-8gev', True )
mySim.generators = [ gen.single_8gev_e_upstream_tagger() ]
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Basic test Simulation'

p.sequence = [ mySim ]
p.run = 1
p.max_events = 10 #new number of electrons
p.output_files = [ 'NewEvents10.root' ] #new output file name

import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.hcal_geometry


