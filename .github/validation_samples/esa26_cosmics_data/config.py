"""Decode ESA 2026 calorimeter raw data with the TS in the readout (run 408)

Skim of run 408 (TS + ECal L1-L3 + HCal, HCal-triggered cosmics at ~400 Hz):
whole events with a TS pulse >= 35 ADC codes and zero-suppressed ECal and HCal
data, all streams kept in file order. The TS raw stream is built but not
decoded. ECal layers 1-3 share one merged ECON-D frame and are split by ECON-D
ID, the HCal backplane comes in its own frame. Five samples per readout,
sample of interest 2, zero suppression on.

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

# frames of one trigger share a timestamp; with the TS in the readout, streams
# reach the file up to ~0.3 s late around readout stalls, so the reorder
# window spans that (a 5e6 window builds only the first ~400 events)
evb = eventbuilder.from_dat_file(
    dat_file,
    verbose_parse=False,
    instance_name="evb",
    coherence_window_ns=10.0,
    reorder_window_ns=1e9,
)
evb.binary_file = ""
evb.performance_file = ""

# ECal (subsystem 5, contributor 40): ECON-D ID 1 = layer 1 (ROCs 0, 1, 3, 4,
# 8 eRx), ID 2 = layer 2 (6 ROCs, 12 eRx), ID 3 = layer 3 (ROCs 0, 2, 3, 6 eRx)
ecal_links = ((1, 8), (2, 12), (3, 6))
ecal_decoders = [
    ECONDDecoder(
        instance_name=f"ecal_l{L}_decoder",
        raw_data_name="ecal",
        digi_output_name=f"EcalL{L}Digis",
        is_ecal=True,
        n_links=n,
        fiber=L,
        econd_id=L,
    )
    for L, n in ecal_links
]
ecal_dqm = [
    ECONDDigiDQM(digi_name=f"EcalL{L}Digis", instance_name=f"ecal_l{L}_dqm")
    for L, _ in ecal_links
]
# layers 2 and 3 again with detector IDs, then rec hits
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
    *ecal_decoders,
    *ecal_dqm,
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
p.run = 408

p.histogram_file = "hist.root"
p.output_files = ["events.root"]
# large raw buffers, only the histograms are compared
p.skim_default_is_drop()

p.logger.term_level = int(os.environ.get("LDMX_LOG_LEVEL", 1))
