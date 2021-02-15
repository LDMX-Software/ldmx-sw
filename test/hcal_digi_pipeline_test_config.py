
from LDMX.Framework import ldmxcfg

# Create a process
p = ldmxcfg.Process( 'test_hcal_digis' )

# Set the maximum number of events
p.maxEvents = 10 # should be the same as NUM_TEST_SIM_HITS

# Import the Hcal conditions 
from LDMX.Hcal import digi

# Set the output file name
p.outputFiles = ['hcal_digi_pipeline_test.root']

# The the histogram file name
p.histogramFile = 'hcal_digi_pipeline_test_histo.root'

# Geometry provider
import LDMX.Hcal.HcalGeometry

# HCal digi
hcalDigis = digi.HcalDigiProducer()

# Turn off noise hits
hcalDigis.hgcroc.noise = False

p.sequence = [
    ldmxcfg.Producer('fakeSimHits','hcal::test::HcalFakeSimHits','Hcal'),
    hcalDigis,
    digi.HcalRecProducer(),
    ldmxcfg.Analyzer('checkHcalHits','hcal::test::HcalCheckEnergyReconstruction','Hcal'),
]

