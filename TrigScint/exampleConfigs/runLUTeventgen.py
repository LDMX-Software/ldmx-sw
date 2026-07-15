#Script 1 of 3 to generate new lookup table for TSpad tracks
#All scripts use local paths and are intended to be executed from the same directory


#Creates simulation sample .root file with TS info, 
#where "SimSamples.root" is then an example input file for use in runClusterstxt.py

from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('simulation')

from LDMX.SimCore import simulator
from LDMX.SimCore import generators as gen

mySim = simulator.Simulator( "mySim" )
mySim.set_detector( 'ldmx-det-v14-8gev', True )
mySim.generators = [ gen.single_8gev_e_upstream_tagger() ]
gen.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Basic test Simulation'

p.sequence = [ mySim ]
p.run = 1 #different for each simulation
p.max_events = 10
p.output_files = [ 'SimSamples.root' ] #new output file name

import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.hcal_geometry
