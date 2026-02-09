from LDMX.Framework import ldmxcfg


# create my process object
p = ldmxcfg.Process( "test" )
# how many events to process?
import sys


p.max_events = 10
if len(sys.argv) > 1 :
    p.max_events = int(sys.argv[1])
# we want to see every event
p.log_frequency = 1
p.term_log_level = 0
# Set a run number
p.run = 9001
# we also only have an output file
p.output_files = [ "justSim_" + str(p.max_events) + "_events.root" ]
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
from LDMX.SimCore import simulator as sim


my_sim = sim.simulator( "my_sim" )
my_sim.setDetector( 'ldmx-det-v14' , include_scoring_planes_minimal = True )
# Get a pre-written generator
from LDMX.SimCore import generators as gen


my_sim.generators.append( gen.single_4gev_e_upstream_tagger() )
# add your configured simulation to the sequence
my_sim.description = 'Basic test Simulation'
p.sequence.append( my_sim )
