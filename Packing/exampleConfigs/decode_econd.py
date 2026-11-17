"""Decode ECON-D event packets from an ESA 2025 raw data file.

Pipeline
--------
    SingleSubsystemUnpacker  ->  {Hcal,Ecal}RawData (bytes)
    ECONDDecoder             ->  {Hcal,Ecal}Digis (HgcrocDigiCollection)
                                 {Hcal,Ecal}DigisECONDInfo (ECONDEventInfo)

Usage
-----
    denv_workspace="$PWD" denv fire Packing/exampleConfigs/decode_econd.py -- \
        /path/to/Run_111_20251212_154633.dat \
        [--subsystem hcal|ecal] [--max-events N] [--output decoded.root]

Run 111 (HCal and ECal with 8 samples per event) is the reference run.
"""

import argparse
import os
import sys

# fire passes '--' through to the script; strip it so argparse sees only flags.
sys.argv = [a for a in sys.argv if a != "--"]

parser = argparse.ArgumentParser(f"ldmx fire {sys.argv[0]}")
parser.add_argument("dat_file", help="raw data file written by Rogue")
parser.add_argument(
    "--subsystem",
    choices=["hcal", "ecal"],
    default="hcal",
    help="subsystem to decode",
)
parser.add_argument(
    "--max-events", type=int, default=100, help="number of events to decode"
)
parser.add_argument("--output", default=None, help="output event file")
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg
from LDMX.Packing import rawio
from LDMX.Packing.decoding import ECONDDecoder

p = ldmxcfg.Process("decode")

prefix = arg.subsystem.capitalize()
unpack = rawio.SingleSubsystemUnpacker(
    dat_file=arg.dat_file,
    output_name=f"{prefix}RawData",
    subsystem_name=arg.subsystem,
)
decode = ECONDDecoder(
    raw_data_name=f"{prefix}RawData",
    digi_output_name=f"{prefix}Digis",
    is_ecal=(arg.subsystem == "ecal"),
)

p.sequence = [unpack, decode]
p.max_events = arg.max_events
p.run = 0
p.output_files = [
    arg.output
    if arg.output is not None
    else os.path.basename(arg.dat_file).replace(".dat", f"_{arg.subsystem}.root")
]
# print the decoding summary at the end
p.logger.custom(decode, level=1)
