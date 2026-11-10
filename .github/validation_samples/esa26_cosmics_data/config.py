"""Decode ESA 2026 calorimeter raw data (run 271)

The EventBuilder groups the TDAQ, ECal, HCal and HCal trigger frames of each
trigger by RoR timestamp. ECal layers 2 and 3 share one merged ECON-D frame
and are split by ECON-D ID, the HCal backplane comes in its own frame.
Five samples per readout, sample of interest 2, zero suppression on.
HCal-triggered cosmics at ~400 Hz.

The raw .dat file is decompressed in init.sh, the EventBuilder takes
whichever run sits in the ci-data directory.
"""

import glob
import os

from LDMX.DQM import dqm
from LDMX.DQM.dqm import EcalRecHitDQM, ECONDDigiDQM
from LDMX.Ecal.digi import EcalRecProducer
from LDMX.Ecal.ecal_detector_map import EcalDetectorMap
from LDMX.Ecal.ecal_geometry import EcalGeometryProvider
from LDMX.Ecal.esa26_conditions import esa26_recon_conditions
from LDMX.EventBuilder import eventbuilder
from LDMX.Framework import ldmxcfg
from LDMX.Packing.decoding import ECONDDecoder


p = ldmxcfg.Process("test")
EcalDetectorMap.esa26()
EcalGeometryProvider.get_instance().set_detector("ldmx-esa-cosmic26-v2")
esa26_recon_conditions()

(dat_file,) = glob.glob(f"{os.environ['CI_DATA']}/esa26_cosmics_data/*.dat")

# frames of one trigger share a timestamp, triggers can be 21 ticks apart and
# contributors arrive up to ~0.5M ticks (2 ms) out of order
evb = eventbuilder.from_dat_file(
    dat_file,
    verbose_parse=False,
    instance_name="evb",
    coherence_window_ns=10.0,
    reorder_window_ns=5e6,
)
evb.binary_file = ""
evb.performance_file = ""

# ECal (subsystem 5, contributor 40): ECON-D ID 2 = layer 2 (6 ROCs, 12 eRx),
# ID 3 = layer 3 (ROCs 0, 2, 3, 6 eRx)
ecal_l2_decoder = ECONDDecoder(
    instance_name="ecal_l2_decoder",
    raw_data_name="ecal",
    digi_output_name="EcalL2Digis",
    is_ecal=True,
    n_links=12,
    fiber=2,
    econd_id=2,
)
ecal_l3_decoder = ECONDDecoder(
    instance_name="ecal_l3_decoder",
    raw_data_name="ecal",
    digi_output_name="EcalL3Digis",
    is_ecal=True,
    n_links=6,
    fiber=3,
    econd_id=3,
)
# same frames with detector IDs, then rec hits
ecal_det = [
    ECONDDecoder(
        instance_name=f"ecal_l{L}_det",
        raw_data_name="ecal",
        digi_output_name=f"EcalL{L}DetDigis",
        is_ecal=True,
        n_links=n,
        fiber=L,
        econd_id=L,
        translate_eid=True,
    )
    for L, n in ((2, 12), (3, 6))
]
ecal_rec = [
    EcalRecProducer.esa_cosmic_stand(
        instance_name=f"ecal_l{L}_rec",
        digi_coll_name=f"EcalL{L}DetDigis",
        digi_pass_name="test",
        rec_hit_coll_name=f"EcalL{L}RecHits",
    )
    for L in (2, 3)
]
ecal_rec_dqm = [
    EcalRecHitDQM(instance_name=f"ecal_l{L}_rec_dqm", rec_hit_coll=f"EcalL{L}RecHits")
    for L in (2, 3)
]
ecal_l2_dqm = ECONDDigiDQM(digi_name="EcalL2Digis", instance_name="ecal_l2_dqm")
ecal_l3_dqm = ECONDDigiDQM(digi_name="EcalL3Digis", instance_name="ecal_l3_dqm")

# HCal (subsystem 7 at ESA26, 5 at ESA25): ROCs 0, 2, 3 (6 eRx)
hcal_decoder = ECONDDecoder(
    instance_name="hcal_decoder",
    raw_data_name="hcal",
    digi_output_name="HcalDigis",
    is_ecal=False,
    n_links=6,
)
hcal_dqm = ECONDDigiDQM(digi_name="HcalDigis", instance_name="hcal_econd_dqm")

p.sequence = [
    evb,
    dqm.EventSummaryDQM(),
    ecal_l2_decoder,
    ecal_l3_decoder,
    ecal_l2_dqm,
    ecal_l3_dqm,
    *ecal_det,
    *ecal_rec,
    *ecal_rec_dqm,
    hcal_decoder,
    hcal_dqm,
]
# one info line per built event otherwise
p.logger.custom("EventBuilder", level=2)

##################################################################
# Below should be the same for all raw data scenarios

p.max_events = int(os.environ["LDMX_NUM_EVENTS"])
# data run number, selects the conditions-data IOV
p.run = 271

p.histogram_file = "hist.root"
p.output_files = ["events.root"]
# large raw buffers, only the histograms are compared
p.skim_default_is_drop()

p.logger.term_level = int(os.environ.get("LDMX_LOG_LEVEL", 1))
