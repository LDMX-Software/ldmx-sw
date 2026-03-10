from LDMX.Framework import ldmxcfg


# Create a process
p = ldmxcfg.Process( 'test_hcal_geometry' )

# Set the maximum number of events
p.max_events = 10

# Set the run number
p.run = 0
# Import the Hcal conditions
from LDMX.Hcal import digi


# Set the output file name
p.output_files = ['hcal_geometry_test.root']

# The histogram file name
p.histogram_file = 'hcal_geometry_test_histo.root'

# Geometry provider
import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.Hcal import hcal_hardcoded_conditions


# HCal digi
hcal_digis = digi.HcalDigiProducer()

# Turn off noise hits
hcal_digis.hgcroc.noise = False

#  Generate muons
from LDMX.SimCore import generators, simulator


sim = simulator.simulator(instance_name = "single_neutron")
sim.setDetector( 'ldmx-det-v14' , False )
sim.description = "HCal muon"

# shoot muon with a general Particle source
n_part=1
gps_cmds=[ "/gps/particle mu-",
          "/gps/pos/type Plane",
          "/gps/direction 0 0 1",
          "/gps/ene/mono 4 GeV", # fixed energy
          "/gps/pos/shape Square",
          "/gps/pos/centre 0 0 220. mm", # start at side hcal
          "/gps/pos/halfx 1000 mm",
          "/gps/pos/halfy 1000 mm",
          "/gps/number "+str(n_part),
          ]
if n_part>1:
     gps_cmds += (n_part-1)*(['/gps/source/add 1']+gps_cmds)
     gps_cmds += ["/gps/source/multiplevertex True"]
my_gps = generators.gps(gps_cmds)
sim.generators.append(my_gps)

p.sequence = [
    sim,
    hcal_digis,
    ldmxcfg.make_processor('hcalpos','hcal::test::HcalCheckPositionMap','Hcal'),
]
