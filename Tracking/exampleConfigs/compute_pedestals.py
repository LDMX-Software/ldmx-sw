"""Compute per-channel pedestal mean and noise from a baseline Rogue run.

This is step 1 of the real-data tracker waveform chain:

    1. compute_pedestals.py       (this script)  -> pedestals.json
    2. raw_to_measurements.py --stop-at fit      -> <output>.root (TrackerWaveforms)
    3. plot_waveforms.py                          -> PNG figures

Runs SingleSubsystemUnpacker -> RawTrackerDecoder -> PedestalCalculator on a
baseline (pedestal) run, computing per-channel, per-sample mean and RMS noise,
and writes them to a JSON file.  Pass that JSON to raw_to_measurements.py for
physics runs.

Usage
-----
    just fire Tracking/exampleConfigs/compute_pedestals.py \
        [-- --dat /path/to/baseline.dat] \
        [-- --output-json pedestals.json] \
        [-- --max-events N]
"""

import argparse
import sys

# just fire passes '--' through to the script; strip it so argparse sees only flags.
sys.argv = [a for a in sys.argv if a != "--"]

parser = argparse.ArgumentParser(f"ldmx fire {sys.argv[0]}")
parser.add_argument(
    "--dat",
    default="/sdf/data/hps/users/mgignac/hardware/data/LDMX/Run_049_20251211_093048.dat",
    help="Path to the baseline (pedestal) Rogue .dat file. Must be a dedicated "
    "pedestal run, NOT a physics run.",
)
parser.add_argument(
    "--output-json",
    default="pedestals.json",
    help="Output path for the pedestal JSON file (default: pedestals.json)",
)
parser.add_argument(
    "--max-events",
    type=int,
    default=500,
    help="Number of baseline frames to accumulate (default: 500)",
)
parser.add_argument(
    "--frame-offset",
    type=int,
    default=0,
    help="Skip this many tracker frames at the start of the file (default: 0)",
)
parser.add_argument(
    "--output",
    default="pedestal_calc.root",
    help=(
        "Output ROOT file (required by the framework; the important output is the JSON)"
    ),
)
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg
from LDMX.Packing import rawio
from LDMX.Tracking import rawdecoder

p = ldmxcfg.Process("pedestalCalc")
p.log_frequency = 50
p.max_events = arg.max_events
p.output_files = [arg.output]

# Stage 1: unpack raw Rogue frames into byte buffers.
unpacker = rawio.SingleSubsystemUnpacker(
    dat_file=arg.dat,
    output_name="TrackerRawData",
    subsystem_name="tracker",
    frame_offset=arg.frame_offset,
)

# Stage 2: decode byte buffers into RawSiStripHit objects.
decoder = rawdecoder.RawTrackerDecoder()
decoder.output_collection = "RawSiStripHits"

# Stage 3: accumulate ADC statistics and write pedestals.json.
calc = rawdecoder.PedestalCalculator()
calc.input_collection = decoder.output_collection
calc.output_file = arg.output_json

p.sequence = [unpacker, decoder, calc]
