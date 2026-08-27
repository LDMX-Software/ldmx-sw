"""Real-data tracker chain: raw bytes -> global-position Measurements.

This is the step that connects the electronics-addressed waveform chain to the
geometry-aware reconstruction, using a DAQ map to turn (feb, hybrid, pchannel)
into (layer_id, strip_id).

The chain can be truncated with --stop-at, which subsumes the old
decode_to_waveforms.py workflows:

    --stop-at waveforms    ->  TrackerWaveforms                     (no fit)
    --stop-at fit          ->  TrackerWaveforms + FittedSiStripHits
    --stop-at measurements ->  ... + StripMeasurements              (default)

The 'fit' output is exactly what plot_waveforms.py reads -- same collection
names, same 'trackerReco' pass -- so waveform QA runs straight off this script
(point plot_waveforms.py at the output ROOT file).

Pipeline
--------
    SingleSubsystemUnpacker  ->  TrackerRawData (bytes)
    RawTrackerDecoder        ->  RawSiStripHits
    PedestalSubtractor       ->  TrackerHits          (pedestal-subtracted)
    SiStripWaveformBuilder      ->  TrackerWaveforms  (SiStripWaveform)
    SiStripWaveformFitProcessor ->  FittedSiStripHits (geometry-addressed)
    StripClusterProcessor       ->  StripMeasurements (global x/y/z)

Pedestals/noise are supplied as a conditions object (TrackerPedestals) by
TrackerPedestalProvider; the DAQ map is a plain JSON file.  Run
compute_pedestals.py first to produce the pedestal JSON.

Usage
-----
    denv_workspace="$PWD" denv fire Tracking/exampleConfigs/raw_to_measurements.py -- \
        [--dat /path/to/physics.dat] \
        [--pedestal-file pedestals.json] \
        [--daq-map /path/to/daqmap.json] \
        [--detector ldmx-reduced-v3] \
        [--max-events N] \
        [--stop-at waveforms|fit|measurements] \
        [--output measurements.root]
"""

import argparse
import sys

# fire passes '--' through to the script; strip it so argparse sees only flags.
sys.argv = [a for a in sys.argv if a != "--"]

parser = argparse.ArgumentParser(f"ldmx fire {sys.argv[0]}")
parser.add_argument(
    "--dat",
    default="/sdf/data/hps/users/mgignac/hardware/data/LDMX/Run_037_20251210_145218.dat",
    help="Path to the physics Rogue .dat file",
)
parser.add_argument(
    "--pedestal-file",
    default="pedestals.json",
    help="Pedestal JSON from compute_pedestals.py",
)
parser.add_argument(
    "--daq-map",
    default=None,
    help="DAQ map JSON (default: the installed ESA slice-test map)",
)
parser.add_argument(
    "--detector",
    default="ldmx-reduced-v3",
    help="Detector name for the tracking geometry (default: ldmx-reduced-v3)",
)
parser.add_argument("--max-events", type=int, default=300)
parser.add_argument("--frame-offset", type=int, default=0)
parser.add_argument("--output", default="measurements.root")
parser.add_argument("--n-triggers", type=int, default=10)
# Waveform-builder significance cuts (both conditions must pass).
parser.add_argument("--high-threshold", type=float, default=5.0)
parser.add_argument("--min-high-samples", type=int, default=4)
parser.add_argument("--low-threshold", type=float, default=3.0)
parser.add_argument("--min-consecutive-low", type=int, default=5)
# Clustering significance cuts.
parser.add_argument("--seed-threshold", type=float, default=4.0)
parser.add_argument("--neighbor-threshold", type=float, default=3.0)
parser.add_argument("--cluster-threshold", type=float, default=4.0)
# Chain truncation (subsumes the old decode_to_waveforms.py --no-fit / QA modes).
parser.add_argument(
    "--stop-at",
    choices=("waveforms", "fit", "measurements"),
    default="measurements",
    help="Truncate the chain: 'waveforms' (only TrackerWaveforms, no fit / no "
    "DAQ map), 'fit' (adds FittedSiStripHits -- what plot_waveforms.py "
    "wants), or 'measurements' (full chain to StripMeasurements, default).",
)
parser.add_argument(
    "--verbose-waveforms",
    action="store_true",
    help="Set the waveform builder (and fitter, if run) to trace logging, "
    "printing per-channel fit results and full ASCII waveform traces.",
)
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg
from LDMX.Packing import rawio
from LDMX.Tracking import rawdecoder, tracking
from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as TrackGeo

daq_map = arg.daq_map if arg.daq_map is not None else rawdecoder.daq_map_path()

p = ldmxcfg.Process("trackerReco")
p.log_frequency = 1
p.max_events = arg.max_events
p.output_files = [arg.output]

# Tracking geometry for the local -> global transform in StripClusterProcessor.
TrackGeo.get_instance().set_detector(arg.detector)

# Stage 1: unpack raw Rogue frames.
unpacker = rawio.SingleSubsystemUnpacker(
    dat_file=arg.dat,
    output_name="TrackerRawData",
    subsystem_name="tracker",
    frame_offset=arg.frame_offset,
)

# Stage 2: decode into RawSiStripHit.
decoder = rawdecoder.RawTrackerDecoder()
decoder.output_collection = "RawSiStripHits"

# Pedestals delivered as a conditions object (service).
peds = rawdecoder.TrackerPedestalProvider(pedestal_file=arg.pedestal_file)

# Stage 3: subtract pedestals.
subtractor = rawdecoder.PedestalSubtractor()
subtractor.input_collection = decoder.output_collection
subtractor.output_collection = "TrackerHits"

# Stage 4: assemble per-channel waveforms.
builder = rawdecoder.SiStripWaveformBuilder()
builder.input_collection = subtractor.output_collection
builder.output_collection = "TrackerWaveforms"
builder.n_triggers = arg.n_triggers
builder.high_threshold = arg.high_threshold
builder.min_high_samples = arg.min_high_samples
builder.low_threshold = arg.low_threshold
builder.min_consecutive_low = arg.min_consecutive_low

# Stage 5: fit the pulse shape and map electronics addresses to
# (layer_id, strip_id) via the DAQ map.
fitter = rawdecoder.SiStripWaveformFitProcessor()
fitter.input_collection = builder.output_collection
fitter.output_collection = "FittedSiStripHits"
fitter.daq_map_file = daq_map

# Stage 6: cluster into global-position Measurements.
clusterer = tracking.StripClusterProcessor()
clusterer.in_collection = fitter.output_collection
clusterer.out_collection = "StripMeasurements"
clusterer.daq_map_file = daq_map
clusterer.seed_threshold = arg.seed_threshold
clusterer.neighbor_threshold = arg.neighbor_threshold
clusterer.cluster_threshold = arg.cluster_threshold

# Assemble the sequence, truncating at --stop-at. The builder always runs; the
# fitter is added for 'fit'/'measurements', the clusterer only for the full
# 'measurements' chain. (In 'waveforms' mode the fitter/clusterer objects above
# are simply left out of the sequence, and the DAQ map is never opened.)
p.sequence = [unpacker, decoder, subtractor, builder]
if arg.stop_at in ("fit", "measurements"):
    p.sequence.append(fitter)
if arg.stop_at == "measurements":
    p.sequence.append(clusterer)

if arg.verbose_waveforms:
    p.logger.trace(builder)
    if arg.stop_at in ("fit", "measurements"):
        p.logger.trace(fitter)
