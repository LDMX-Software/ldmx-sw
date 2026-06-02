"""Basic HGCROCv2RawDataFile reformatting configuration"""

import argparse
import sys


parser = argparse.ArgumentParser(f"ldmx fire {sys.argv[0]}")

parser.add_argument("output_file")

parser.add_argument("--wr")
parser.add_argument("--ts")
parser.add_argument("--ft41")
parser.add_argument("--ft42")
parser.add_argument("--ft50")
parser.add_argument("--ft51")
parser.add_argument("--pf0")
parser.add_argument("--pf1")
parser.add_argument("--hcal")

parser.add_argument("--max_events", default=100, type=int)
parser.add_argument("--pause", action="store_true")
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("unpack")
p.max_events = arg.max_events
p.term_log_level = 1
p.log_frequency = 1

import LDMX.Hcal.hgcrocFormat as HcalFormat
import LDMX.TrigScint.qieFormat as TsFormat
from LDMX.DQM import dqm
from LDMX.Packing import rawio


p.output_files = [arg.output_file]

# where the ntuplizing tree will go
import os


p.histogram_file = (
    f"{os.path.dirname(arg.output_file)}ntuple_{os.path.basename(arg.output_file)}"
)

if arg.wr is not None:
    p.sequence.append(rawio.WRRawDecoder(input_file=arg.wr, output_name="WRRaw"))

if arg.ft41 is not None:
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            input_file=arg.ft41, instance_name="ft41", output_name="FiberTracker41Raw"
        )
    )

if arg.ft42 is not None:
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            input_file=arg.ft42, instance_name="ft42", output_name="FiberTracker42Raw"
        )
    )

if arg.ft50 is not None:
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            input_file=arg.ft50, instance_name="ft50", output_name="FiberTracker50Raw"
        )
    )

if arg.ft51 is not None:
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            input_file=arg.ft51, instance_name="ft51", output_name="FiberTracker51Raw"
        )
    )

if arg.pf0 is not None:
    p.input_files = [arg.pf0]
    p.sequence.extend(
        [
            HcalFormat.HcalRawDecoder(
                input_names=["Polarfire0Raw"], input_pass="raw", output_name="PF0Digis"
            ),
            dqm.NtuplizeHgcrocDigiCollection(
                input_name="PF0Digis", instance_name="pf0"
            ),
        ]
    )

if arg.pf1 is not None:
    p.input_files = [arg.pf1]
    p.sequence.extend(
        [
            HcalFormat.HcalRawDecoder(
                input_names=["Polarfire1Raw"], input_pass="raw", output_name="PF1Digis"
            ),
            dqm.NtuplizeHgcrocDigiCollection(
                input_name="PF1Digis", instance_name="pf1"
            ),
        ]
    )

if arg.hcal is not None:
    p.input_files = [arg.hcal]
    p.sequence.extend(
        [
            HcalFormat.HcalRawDecoder(
                connections_table=(
                    f"{os.environ['LDMX_BASE']}"
                    "/ldmx-sw/Hcal/data/testbeam_connections.csv"
                ),
                input_names=["Polarfire0Raw", "Polarfire1Raw"],
                input_pass="raw",
                output_name="HcalDigis",
            ),
            dqm.NtuplizeHgcrocDigiCollection(
                input_name="HcalDigis", instance_name="hcal"
            ),
        ]
    )

if arg.ts is not None:
    p.input_files = [arg.ts]
    n_channels = 16
    n_timesamples = 30
    header_len = 4 + 4 + 4 + 3 + 1
    qie_decoder = TsFormat.QIEDecoder.up(
        os.environ["LDMX_BASE"]
        + "/ldmx-sw/TrigScint/data/"
        + "channelMap_LYSOback_plasticFront_12-to-16channels_rotated180.txt"
    )
    qie_decoder.number_channels = n_channels
    qie_decoder.number_time_samples = n_timesamples
    qie_decoder.is_real_data = True
    qie_decoder.input_collection = "TrigScintRaw"
    p.sequence.extend(
        [qie_decoder, dqm.NtuplizeTrigScintQIEDigis(input_name="decodedQIEUp")]
    )

if arg.pause:
    p.pause()
