from LDMX.Ecal import digi, ecal_geometry, ecal_hardcoded_conditions, ecal_trig_digi
from LDMX.Framework import ldmxcfg


# Create a process
p = ldmxcfg.Process("test_ecal_digis")

# Set the maximum number of events
p.max_events = 2000


# Set the output file name
p.output_files = ["ecal_digi_pipeline_test.root"]

# The the histogram file name
p.histogram_file = "ecal_digi_pipeline_test_histo.root"

geom = ecal_geometry.EcalGeometryProvider.get_instance()

# ECal digi
ecal_digis = digi.EcalDigiProducer(
    mev=digi.calculate_energy_to_voltage_conversion(si_thickness=0.5)
)

# Turn of noise hits
ecal_digis.hgcroc.noise = False

p.sequence = [
    ldmxcfg.make_processor("fakeSimHits", "ecal::test::EcalFakeSimHits", "Ecal"),
    ecal_digis,
    ecal_trig_digi.EcalTrigPrimDigiProducer(),
    digi.EcalRecProducer(),
    ldmxcfg.make_processor(
        "checkEcalHits", "ecal::test::EcalCheckEnergyReconstruction", "Ecal"
    ),
]
