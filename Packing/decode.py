"""Basic HGCROCv2RawDataFile reformatting configuration"""

import argparse, sys

parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}')

parser.add_argument('output_file')

parser.add_argument('--wr',required=True)
parser.add_argument('--ft',required=True)
parser.add_argument('--pf0',required=True)
parser.add_argument('--pf1',required=True)

parser.add_argument('--max_events',default=100,type=int)
parser.add_argument('--pause',action='store_true')
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack')
p.maxEvents = arg.max_events
p.termLogLevel = 0
p.logFrequency = 100

import LDMX.Hcal.hgcrocFormat as hcal_format
from LDMX.DQM import dqm
from LDMX.Packing import rawio

p.outputFiles = [arg.output_file]

# where the ntuplizing tree will go
import os
p.histogramFile = f'{os.path.dirname(arg.output_file)}ntuple_{os.path.basename(arg.output_file)}'

# sequence
p.sequence = [ 
        rawio.WRRawDecoder(
            raw_file = arg.wr,
            output_name = 'WRRaw'),
        rawio.FiberTrackerRawDecoder(
            raw_file = arg.ft,
            output_name = 'FiberTrackerRaw'),
        hcal_format.HcalRawDecoder(
            input_file = arg.pf0,
            output_name = 'PF0Raw'
            ),
        dqm.NtuplizeHgcrocDigiCollection(
            input_name = 'PF0Raw',
            name = 'pf0'
            ),
        hcal_format.HcalRawDecoder(
            input_file = arg.pf1,
            output_name = 'PF1Raw'
            ),
        dqm.NtuplizeHgcrocDigiCollection(
            input_name = 'PF1Raw',
            name = 'pf1'
            ),
        ]

if arg.pause :
    p.pause()
