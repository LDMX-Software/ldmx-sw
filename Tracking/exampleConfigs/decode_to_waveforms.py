"""Decode tracker Rogue data, subtract pedestals, and build per-channel waveforms.

This is step 2 of the real-data tracker waveform chain:

    1. compute_pedestals.py                   -> pedestals.json
    2. decode_to_waveforms.py  (this script)  -> <output>.root (TrackerWaveforms)
    3. plot_waveforms.py                      -> PNG figures

Pipeline
--------
    SingleSubsystemUnpacker  ->  TrackerRawData (bytes)
    RawTrackerDecoder        ->  RawSiStripHits
    PedestalSubtractor          ->  TrackerHits       (RawSiStripHit, subtracted)
    SiStripWaveformBuilder      ->  TrackerWaveforms  (SiStripWaveform)
    SiStripWaveformFitProcessor ->  FittedSiStripHits (fit results)

Pedestals/noise are supplied as a conditions object (TrackerPedestals) by
TrackerPedestalProvider, not stored in the hits.  Run compute_pedestals.py first
to produce the pedestal JSON, then pass it here via --pedestal-file (it is handed
to the provider).  The output ROOT file contains a SiStripWaveform collection
('TrackerWaveforms') ready for waveform analysis / plotting.

The pulse fit lives in a separate processor and its results are written to a
FittedSiStripHit collection, not stored on the waveform.  It is run here so that
plot_waveforms.py can overlay the fitted pulse on the samples; pass --no-fit to
skip it (the overlay is then simply absent from the plots).

Usage
-----
    ldmx fire Tracking/exampleConfigs/decode_to_waveforms.py \
        [-- --dat /path/to/physics.dat] \
        [-- --pedestal-file pedestals.json] \
        [-- --max-events N] \
        [-- --output tracker_waveforms.root]
"""

import argparse
import sys

# just fire passes '--' through to the script; strip it so argparse sees only flags.
sys.argv = [a for a in sys.argv if a != '--']

parser = argparse.ArgumentParser(f"ldmx fire {sys.argv[0]}")
parser.add_argument(
    "--dat",
    default="/sdf/data/hps/users/mgignac/hardware/data/LDMX/Run_037_20251210_145218.dat",
    help="Path to the physics Rogue .dat file",
)
parser.add_argument(
    "--pedestal-file",
    default="pedestals.json",
    help="Path to the pedestal JSON file from compute_pedestals.py",
)
parser.add_argument(
    "--max-events",
    type=int,
    default=10,
    help="Maximum number of physics events to process (default: 10)",
)
parser.add_argument(
    "--frame-offset",
    type=int,
    default=0,
    help="Skip this many tracker frames at the start of the file (default: 0)",
)
parser.add_argument(
    "--output",
    default="tracker_waveforms.root",
    help="Output ROOT file (default: tracker_waveforms.root)",
)
parser.add_argument(
    "--verbose-waveforms",
    action="store_true",
    help="Drop the waveform builder to the trace logging level, printing the "
    "per-channel fit results and full ASCII waveform traces",
)
parser.add_argument("--high-threshold", type=float, default=5.0,
                    help="Per-sample significance (ADC/noise) for the high-threshold count cut")
parser.add_argument("--min-high-samples", type=int, default=4,
                    help="Min samples that must exceed high_threshold")
parser.add_argument("--low-threshold", type=float, default=3.0,
                    help="Per-sample significance (ADC/noise) for the consecutive-streak cut")
parser.add_argument("--min-consecutive-low", type=int, default=5,
                    help="Min consecutive samples that must exceed low_threshold")
parser.add_argument("--n-triggers", type=int, default=10,
                    help="Expected number of APV triggers per RoR")
parser.add_argument("--daq-map", default=None,
                    help="DAQ map JSON (default: the installed ESA slice-test map)")
parser.add_argument("--no-fit", action="store_true",
                    help="Skip the pulse-fit stage; only TrackerWaveforms is written")
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg
from LDMX.Packing import rawio
from LDMX.Tracking import rawdecoder

p = ldmxcfg.Process("trackerReco")
p.log_frequency = 1
p.max_events = arg.max_events
p.output_files = [arg.output]

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

# Pedestals are delivered as a conditions object (a service): the provider reads
# the JSON written by compute_pedestals.py and serves it to the processors below.
peds = rawdecoder.TrackerPedestalProvider(pedestal_file=arg.pedestal_file)

# Stage 3: subtract pedestals -> subtracted RawSiStripHit collection.
subtractor = rawdecoder.PedestalSubtractor()
subtractor.input_collection  = decoder.output_collection
subtractor.output_collection = "TrackerHits"

# Stage 4: assemble per-channel waveforms -> SiStripWaveform collection.
builder = rawdecoder.SiStripWaveformBuilder()
builder.input_collection     = subtractor.output_collection
builder.output_collection    = "TrackerWaveforms"
builder.high_threshold       = arg.high_threshold
builder.min_high_samples     = arg.min_high_samples
builder.low_threshold        = arg.low_threshold
builder.min_consecutive_low  = arg.min_consecutive_low
builder.n_triggers           = arg.n_triggers

p.sequence = [unpacker, decoder, subtractor, builder]

# Stage 5: fit the pulse shape -> FittedSiStripHit collection.  Addressing comes
# from the DAQ map, so plot_waveforms.py can join the fits back onto the
# waveforms by inverting the same strip transform.
if not arg.no_fit:
    daq_map = arg.daq_map if arg.daq_map is not None else rawdecoder.daq_map_path()
    fitter = rawdecoder.SiStripWaveformFitProcessor()
    fitter.input_collection = builder.output_collection
    fitter.output_collection = "FittedSiStripHits"
    fitter.daq_map_file = daq_map
    p.sequence.append(fitter)

if arg.verbose_waveforms:
    p.logger.trace(builder)
    if not arg.no_fit:
        p.logger.trace(fitter)
