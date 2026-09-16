from LDMX.Ecal import digi, ecal_geometry, ecal_hardcoded_conditions, ecal_trig_digi
from LDMX.Framework import ldmxcfg


# Create a process
p = ldmxcfg.Process("test_ecal_out_of_time_digis")

# Set the maximum number of events
p.max_events = 2000

# Set the output file name
p.output_files = ["ecal_digi_out_of_time_test.root"]

# The the histogram file name
p.histogram_file = "ecal_digi_out_of_time_test_histo.root"

geom = ecal_geometry.EcalGeometryProvider.get_instance()

# ECal digi
ecal_digis = digi.EcalDigiProducer(
    mev=digi.calculate_energy_to_voltage_conversion(si_thickness=0.5)
)

# Turn of noise hits
ecal_digis.hgcroc.noise = False

# A single, always the same, hit of ~200 MIPs. This is well above the ~50 MIP
# TOT threshold and well below saturation, so it is always read out in TOT mode
# no matter when it arrives.
energy_of_hit = 200 * digi.mip_si_energy

# Scan the arrival time of that hit across the bunch crossings the chip can
# report a TOT measurement in. The chip decides whether such a hit is read out;
# what we require is that the ones it does read out are reconstructed.
fake_sim_hits = ldmxcfg.make_processor(
    "fakeSimHits",
    "ecal::test::EcalFakeSimHits",
    "Ecal",
    min_energy=energy_of_hit,
    max_energy=energy_of_hit,
    min_time=-30.0,
    max_time=45.0,
)

p.sequence = [
    fake_sim_hits,
    ecal_digis,
    ecal_trig_digi.EcalTrigPrimDigiProducer(),
    digi.EcalRecProducer(),
    ldmxcfg.make_processor(
        "checkEcalHits",
        "ecal::test::EcalCheckEnergyReconstruction",
        "Ecal",
        # the trigger primitives only look at the in-time sample
        check_trig_prim=False,
    ),
]
