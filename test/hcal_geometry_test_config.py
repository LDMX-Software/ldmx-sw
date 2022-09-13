from LDMX.Framework import ldmxcfg

# Create a process
p = ldmxcfg.Process( 'test_hcal_geometry' )

# Set the maximum number of events
p.maxEvents = 1

# Set the run number
p.run = 0
# Import the Hcal conditions 
from LDMX.Hcal import digi

# Set the output file name
p.outputFiles = ['hcal_geometry_test.root']

# The histogram file name
p.histogramFile = 'hcal_geometry_test_histo.root'

# Geometry provider
import LDMX.Hcal.HcalGeometry
import LDMX.Ecal.EcalGeometry
from LDMX.Hcal import hcal_hardcoded_conditions

# HCal digi
hcalDigis = digi.HcalDigiProducer()

# Turn off noise hits
hcalDigis.hgcroc.noise = False

#  Generate muons
from LDMX.SimCore import simulator
from LDMX.SimCore import generators

sim = simulator.simulator("single_neutron")
sim.setDetector( 'ldmx-det-v12' , True )
sim.description = "HCal muon"
sim.beamSpotSmear = [20., 80., 0.] #mm

# shoot muon with a general Particle source
nPart=1
gpsCmds=[ "/gps/particle mu-",
          "/gps/pos/type Plane",
          "/gps/direction 0 0 1",
          "/gps/ene/mono 4 GeV", # fixed energy
          "/gps/pos/shape Square",
          "/gps/pos/centre 0 0 220. mm", # start at side hcal
          "/gps/pos/halfx 1500 mm",
          "/gps/pos/halfy 1500 mm",
          "/gps/number "+str(nPart),
          ]
if nPart>1:
     gpsCmds += (nPart-1)*(['/gps/source/add 1']+gpsCmds)
     gpsCmds += ["/gps/source/multiplevertex True"]
myGPS = generators.gps( 'myGPS' , gpsCmds )
sim.generators.append(myGPS)

p.sequence = [
    sim,
    hcalDigis,
    ldmxcfg.Analyzer('hcalpos','hcal::test::HcalCheckPositionMap','Hcal'),
]
