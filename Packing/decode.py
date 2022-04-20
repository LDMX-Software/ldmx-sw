"""Basic HGCROCv2RawDataFile reformatting configuration"""

import argparse, sys

parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}')

parser.add_argument('output_file')

parser.add_argument('--wr',required=True)

parser.add_argument('--max_events',default=100,type=int)
parser.add_argument('--pause',action='store_true')
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack')
p.maxEvents = arg.max_events
p.termLogLevel = 0
p.logFrequency = 1

import LDMX.Hcal.hgcrocFormat as hcal_format
import LDMX.Hcal.digi as hcal_digi
import LDMX.Hcal.hcal_hardcoded_conditions
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
            output_name = 'WRRaw')
        ]

if arg.pause :
    p.pause()
