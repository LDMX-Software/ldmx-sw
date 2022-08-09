from LDMX.Framework import ldmxcfg
# create my process object
p = ldmxcfg.Process( "test" )
p.run = 9001
p.maxEvents = 10
p.outputFiles = [ "energy_sort.root" ]
from LDMX.SimCore import simulator as sim
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v12' )
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
mySim.description = 'Basic test Simulation'
from LDMX.Biasing import util
mySim.actions = [ util.PartialEnergySorter( 3000. ) ]
p.sequence.append( mySim )
