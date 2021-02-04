from LDMX.Framework import ldmxcfg
# create my process object
p = ldmxcfg.Process( "test" )
p.maxEvents = 10
p.outputFiles = [ "/tmp/energy_sort.root" ]
from LDMX.SimCore import simulator as sim
from LDMX.Ecal import EcalGeometry
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v12' )
mySim.runNumber = 9001
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
mySim.description = 'Basic test Simulation'
mySim.randomSeeds = [ 1 , 2 ]
from LDMX.Biasing import util
mySim.actions = [ util.PartialEnergySorter( 3000. ) ]
p.sequence.append( mySim )
