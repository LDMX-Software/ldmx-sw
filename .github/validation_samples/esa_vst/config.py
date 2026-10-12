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
from LDMX.Tracking import rawdecoder, tracking
from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as TrackGeo
from LDMX.Tracking.ts_measurements import (
    TrigScintMeasurementProducer,
    ts_cluster_chain,
)
from LDMX.TrigScint.trigscint_geometry import TrigScintGeometryProvider
from LDMX.TrigScint.zccm_format import ZCCMDecoder


# Tracking geometry for the local -> global transform in StripClusterProcessor.
TrackGeo.get_instance().set_detector("ldmx-esa25-v1")

# TS geometry conditions provider (bar positions for TrigScintMeasurementProducer),
# pinned to ESA25 since real-data RunHeaders carry no detector name.
TrigScintGeometryProvider.get_instance().set_detector("ldmx-esa25-v1")


# Tracker: raw frames -> strip hits -> pedestal subtracted -> waveforms -> fits
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
trk_fits = rawdecoder.SiStripWaveformFitProcessor(
    daq_map_file=rawdecoder.daq_map_path()
)
trk_dqm = tracking_dqm.RawSiStripDQM()
# geometry-aware clustering: FittedSiStripHits -> global-position StripMeasurements
trk_clusters = tracking.StripClusterProcessor()
trk_clusters.daq_map_file = rawdecoder.daq_map_path()

# TS: raw frames -> ZCCM decoding -> standard rechit + cluster reconstruction.
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
# Standard ESA25 TS reconstruction of the three plastic pads:
#   decodedZCCMPad{1,2,3} -> TrigScintRecHitProducer -> TrigScintClusterProducer
ts_calib_files = {p: f"{ts_data}/esa25/calibration_pad{p}.txt" for p in (1, 2, 3)}
ts_reco, ts_cluster_collections = ts_cluster_chain(
    pads=(1, 2, 3), calib_files=ts_calib_files
)
# Geometry-aware TS measurements (global y at the cluster centroid). Exercises the
# TrigScintGeometry class + TrigScintGeometryProvider + TrigScintMeasurementProducer
ts_measurements = TrigScintMeasurementProducer()
ts_measurements.input_pass = ""  # clusters are produced in this same process
ts_measurements.input_collections = ts_cluster_collections

p.sequence = [
    trk_unpack,
    trk_decoder,
    trk_peds,
    trk_waveforms,
    trk_fits,
    trk_dqm,
    trk_clusters,
    ts_unpack,
    ts_decoder,
    *ts_reco,
    ts_measurements,
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
