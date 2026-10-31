"""Decode ESA 2026 calorimeter raw data (run 212)

The EventBuilder groups the TDAQ, ECal and HCal frames of each trigger by
RoR timestamp. ECal layers 2 and 3 share one merged ECON-D frame and are
split by ECON-D ID, the HCal backplane comes in its own frame.
Five samples per readout, sample of interest 2.

The raw .dat file is decompressed in init.sh, the EventBuilder takes
whichever run sits in the ci-data directory.
"""

import glob
import os

from LDMX.DQM import dqm
from LDMX.DQM.dqm import ECONDDigiDQM
from LDMX.EventBuilder import eventbuilder
from LDMX.Framework import ldmxcfg
from LDMX.Packing.decoding import ECONDDecoder


p = ldmxcfg.Process("test")

(dat_file,) = glob.glob(f"{os.environ['CI_DATA']}/esa26_cosmics_data/*.dat")

evb = eventbuilder.from_dat_file(dat_file, verbose_parse=False, instance_name="evb")
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
ecal_l2_dqm = ECONDDigiDQM(digi_name="EcalL2Digis", instance_name="ecal_l2_dqm")
ecal_l3_dqm = ECONDDigiDQM(digi_name="EcalL3Digis", instance_name="ecal_l3_dqm")

# HCal: ROCs 0, 2, 3 (6 eRx). At ESA26 it is subsystem 7 (5 at ESA25), which
# has no name yet, so the EventBuilder names it subsystem7c20
hcal_decoder = ECONDDecoder(
    instance_name="hcal_decoder",
    raw_data_name="subsystem7c20",
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
    hcal_decoder,
    hcal_dqm,
]
# one info line per built event otherwise
p.logger.custom("EventBuilder", level=2)

##################################################################
# Below should be the same for all raw data scenarios

p.max_events = int(os.environ["LDMX_NUM_EVENTS"])
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist.root"
p.output_files = ["events.root"]
# large raw buffers, only the histograms are compared
p.skim_default_is_drop()

p.logger.term_level = int(os.environ.get("LDMX_LOG_LEVEL", 1))
