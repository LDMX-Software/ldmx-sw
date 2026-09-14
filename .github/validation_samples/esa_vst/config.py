"""Decode and reconstruct ESA 2025 vertical slice test raw data (run 111)

The raw .dat file is assembled from compressed parts in init.sh.
"""

import os

from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")

dat_file = f"{os.environ['CI_DATA']}/esa_vst/run111.dat"
ts_data = os.path.realpath("../../../TrigScint/data")

from LDMX.Packing import rawio
from LDMX.Tracking import dqm as tracking_dqm
from LDMX.Tracking import rawdecoder
from LDMX.TrigScint.trig_scint import (
    EventReadoutProducer,
    QIEAnalyzer,
    TestBeamClusterProducer,
    TestBeamHitProducer,
)
from LDMX.TrigScint.zccm_format import ZCCMDecoder


# Tracker: raw frames -> strip hits -> pedestal subtracted -> waveforms
trk_unpack = rawio.SingleSubsystemUnpacker(
    instance_name="trk_unpack",
    dat_file=dat_file,
    output_name="TrackerRawData",
    subsystem_name="tracker",
)
trk_decoder = rawdecoder.RawTrackerDecoder()
rawdecoder.TrackerPedestalProvider(pedestal_file="pedestals.json")
trk_peds = rawdecoder.PedestalSubtractor()
trk_waveforms = rawdecoder.SiStripWaveformBuilder()
trk_dqm = tracking_dqm.RawSiStripDQM()

# TS: raw frames -> ZCCM decoding -> QIE samples -> hits -> clusters
n_ts_channels = 24
ts_unpack = rawio.SingleSubsystemUnpacker(
    instance_name="ts_unpack",
    dat_file=dat_file,
    output_name="ZCCMoutput",
    subsystem=2,
    contributor=1,
)
ts_decoder = ZCCMDecoder(
    channel_map_file=f"{ts_data}/channelMap_4modules_14lanes.txt",
    module_map_file=f"{ts_data}/moduleMap_4modules_14lanes.txt",
    output_collection="decodedZCCMPad",
    number_time_samples=30,
)
ts_readout = EventReadoutProducer("ts_readout")
ts_readout.input_collection = "decodedZCCMPad1"
ts_readout.time_shift = 0
ts_hits = TestBeamHitProducer("ts_hits")
ts_hits.pedestals = [-2.0] * n_ts_channels
ts_hits.gain = [2e6] * n_ts_channels
ts_hits.start_sample = 10
ts_hits.pulse_width_lyso = 5
ts_hits.n_instrumented_channels = n_ts_channels
ts_clusters = TestBeamClusterProducer("ts_clusters")
ts_clusters.pad_time = 0.0
ts_clusters.clustering_threshold = 15.0

ts_qie_ana = QIEAnalyzer("ts_qie_ana")
ts_qie_ana.start_sample = 0
ts_qie_ana.pedestals = [2.0] * n_ts_channels
ts_qie_ana.gain = [2e6] * n_ts_channels
# per-event displays would add 4800 histograms to compare
ts_qie_ana.n_event_displays = 0

p.sequence = [
    trk_unpack,
    trk_decoder,
    trk_peds,
    trk_waveforms,
    trk_dqm,
    ts_unpack,
    ts_decoder,
    ts_readout,
    ts_hits,
    ts_clusters,
    ts_qie_ana,
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
# prints every fired TDC at info level
p.logger.custom(ts_qie_ana, level=2)
