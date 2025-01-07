"""Run TrigPrimResolutionAnalyzer over an already-created file

    ldmx fire trig-prim-analyzer.py --help
"""

import argparse
from pathlib import Path

parser = argparse.ArgumentParser()

parser.add_argument('--max-events',default=None,type=int,help='maximum number of events to process (helpful to limit while test-running)')
parser.add_argument('--out-dir',default=Path.cwd(),help='directory to put output histogram file')
parser.add_argument('--out-name',default=None,help='name output histogram file (default is to deduce a name from the input file)')
parser.add_argument('input_file',nargs='+',type=Path,help='input file(s) to run analyzer over')

arg = parser.parse_args()

if len(arg.input_file) > 1 and arg.out_name is None:
    parser.error('Need to give --out-name if more than one input file is given.')
elif arg.out_name is None:
    arg.out_name = arg.input_file[0].stem
    if 'type_events' in arg.out_name:
        arg.out_name = arg.out_name.replace('events','histos')
    else:
        arg.out_name += '_histos'
    arg.out_name += '.root'

if not arg.out_dir.is_dir():
    parser.error(f'Output directory {arg.out_dir} does not exist. Is it mounted to the container? Do you need to create it?')

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process( "valid" )
if arg.max_events is not None:
    p.maxEvents = arg.max_events

p.inputFiles = [str(path) for path in arg.input_file]
p.histogramFile = str(arg.out_dir / arg.out_name)

# print updates every 1k events
p.logFrequency = 1000
p.termLogLevel = 0

import LDMX.Ecal.EcalGeometry

p.sequence = [
    ldmxcfg.Analyzer('resolution','ldmx::ecal::TrigPrimResolutionAnalyzer','Ecal')
]
