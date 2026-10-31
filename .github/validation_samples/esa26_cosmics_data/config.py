"""Decode ESA 2026 calorimeter raw data (run 209)

ECal layers 2 and 3 share one merged ECON-D frame and are split by
ECON-D ID, the HCal backplane comes in its own frame.
Five samples per readout, sample of interest 2.

The raw .dat file is decompressed in init.sh, the decoders take
whichever run sits in the ci-data directory.
"""

import glob
import os

from LDMX.DQM.dqm import ECONDDigiDQM
from LDMX.Framework import ldmxcfg
from LDMX.Packing import rawio
from LDMX.Packing.decoding import ECONDDecoder


p = ldmxcfg.Process("test")

(dat_file,) = glob.glob(f"{os.environ['CI_DATA']}/esa26_cosmics_data/*.dat")

# ECal: one frame per event, ECON-D ID 2 = layer 2 (6 ROCs, 12 eRx),
# ID 3 = layer 3 (ROCs 0, 2, 3, 6 eRx)
ecal_unpack = rawio.SingleSubsystemUnpacker(
    instance_name="ecal_unpack",
    dat_file=dat_file,
    output_name="EcalRawData",
    subsystem_name="ecal",
    # run 209: the ECal starts with an empty frame, skip it to pair with the HCal
    frame_offset=1,
)
ecal_l2_decoder = ECONDDecoder(
    instance_name="ecal_l2_decoder",
    raw_data_name="EcalRawData",
    digi_output_name="EcalL2Digis",
    is_ecal=True,
    n_links=12,
    fiber=2,
    econd_id=2,
)
ecal_l3_decoder = ECONDDecoder(
    instance_name="ecal_l3_decoder",
    raw_data_name="EcalRawData",
    digi_output_name="EcalL3Digis",
    is_ecal=True,
    n_links=6,
    fiber=3,
    econd_id=3,
)
ecal_l2_dqm = ECONDDigiDQM(digi_name="EcalL2Digis", instance_name="ecal_l2_dqm")
ecal_l3_dqm = ECONDDigiDQM(digi_name="EcalL3Digis", instance_name="ecal_l3_dqm")

# HCal: ROCs 0, 2, 3 (6 eRx), subsystem 7 at ESA26 (5 at ESA25)
hcal_unpack = rawio.SingleSubsystemUnpacker(
    instance_name="hcal_unpack",
    dat_file=dat_file,
    output_name="HcalRawData",
    subsystem=7,
    contributor=20,
)
hcal_decoder = ECONDDecoder(
    instance_name="hcal_decoder",
    raw_data_name="HcalRawData",
    digi_output_name="HcalDigis",
    is_ecal=False,
    n_links=6,
)
hcal_dqm = ECONDDigiDQM(digi_name="HcalDigis", instance_name="hcal_econd_dqm")

p.sequence = [
    ecal_unpack,
    ecal_l2_decoder,
    ecal_l3_decoder,
    ecal_l2_dqm,
    ecal_l3_dqm,
    hcal_unpack,
    hcal_decoder,
    hcal_dqm,
]

##################################################################
# Below should be the same for all raw data scenarios

p.max_events = int(os.environ["LDMX_NUM_EVENTS"])
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist.root"
p.output_files = ["events.root"]
# large raw buffers, only the histograms are compared
p.skim_default_is_drop()

p.logger.term_level = int(os.environ.get("LDMX_LOG_LEVEL", 1))
