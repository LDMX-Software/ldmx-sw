
from LDMX.Framework import ldmxcfg


# Create a process
p = ldmxcfg.Process( 'test_hcal_digis' )

# Set the maximum number of events
p.max_events = 1000 # should be the same as NUM_TEST_SIM_HITS

# Import the Hcal conditions
from LDMX.Hcal import digi, hcal_hardcoded_conditions


# Set the output file name
p.output_files = ['hcal_digi_pipeline_test.root']

# The the histogram file name
p.histogram_file = 'hcal_digi_pipeline_test_histo.root'

# Geometry provider
import LDMX.Hcal.HcalGeometry


# HCal digi
hcal_digis = digi.HcalDigiProducer()
hcal_digis.input_coll_name = 'HcalFakeSimHits'
# Turn off noise hits
hcal_digis.hgcroc.noise = False

hcal_rec = digi.HcalRecProducer()
hcal_rec.sim_hit_coll_name = 'HcalFakeSimHits'

p.sequence = [
    ldmxcfg.Producer('fakeSimHits','hcal::test::HcalFakeSimHits','Hcal'),
    hcal_digis,
    digi.HcalRecProducer(),
    ldmxcfg.Analyzer('checkHcalHits','hcal::test::HcalCheckReconstruction','Hcal'),
]

