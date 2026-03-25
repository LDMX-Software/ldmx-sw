from LDMX.Framework import ldmxcfg


# create my process object
p = ldmxcfg.Process( "test" )
p.run = 9001
p.max_events = 10
p.output_files = [ "energy_sort.root" ]
import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.SimCore import simulator as sim


my_sim = sim.simulator( instance_name="my_sim" )
my_sim.setDetector( 'ldmx-det-v14' )
from LDMX.SimCore import generators as gen


my_sim.generators.append( gen.single_4gev_e_upstream_tagger() )
my_sim.description = 'Basic test Simulation'
from LDMX.Biasing import util


my_sim.actions = [ util.PartialEnergySorter( 3000. ) ]
p.sequence.append( my_sim )
