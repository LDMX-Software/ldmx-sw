"""Pedestal file extraction, currently for testbeam"""

import argparse
import sys

parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}')

parser.add_argument('input_file')
parser.add_argument('--make_histos',action='store_true',help="Make channel-by-channel histogram files")
parser.add_argument('--calib_file',required=True,help="Calibration output file")
parser.add_argument('--max_events',default=10000,type=int)

arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('hcal_ana')
p.max_events = arg.max_events
p.term_log_level = 0
p.log_frequency = 10

import LDMX.Hcal.hgcrocFormat as hcal_format
import LDMX.Hcal.hcal_ana as hcal_ana
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
from LDMX.Packing import rawio

import os
base_name = os.path.basename(arg.input_file).replace('.raw','')

p.output_files = ['/dev/null']

p.histogram_file = f'hist_{base_name}.root'
tbl = f'{os.environ["LDMX_BASE"]}/ldmx-sw/Hcal/data/testbeam_connections.csv'

# sequence
#   1. decode event packet into digi collection
#   2. process pedestals
p.sequence = [
        hcal_format.HcalRawDecoder(
            input_file = arg.input_file,
            connections_table = tbl,
            output_name = 'polarfireDigis'
            ),
        hcal_ana.HcalPedestalAnalyzer(
            input_name = 'polarfireDigis',
            output_file = arg.calib_file,
            make_histos = arg.make_histos,
            comments = "Source file "+arg.input_file
            )
        ]

