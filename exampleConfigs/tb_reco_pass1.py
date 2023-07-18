import argparse, sys, os, pathlib

"""
Takes raw data file after reformat (with run number)
and runs single ended reconstruction
"""

parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}')
parser.add_argument('input_file')
parser.add_argument('--pause',action='store_true')
grp = parser.add_mutually_exclusive_group()
parser.add_argument('--output_dir',default='.',type=pathlib.Path)
#parser.add_argument('--max_events',default=-1,type=int)
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('')
#p.maxEvents = arg.max_events
p.termLogLevel = 0
p.logFrequency = 1000

print(arg.output_dir)

import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_testbeam0422_conditions
import LDMX.Hcal.digi as hcal_digi
import LDMX.Hcal.hgcrocFormat as hcal_format

base_name = os.path.basename(arg.input_file).replace('.root','')
dir_name  = os.path.dirname(arg.output_dir)

p.inputFiles = [arg.input_file]
p.outputFiles = [f'{dir_name}/reco_{base_name}_pass1.root']

# sequence
tbl = f'{os.environ["LDMX_BASE"]}/ldmx-sw/Hcal/data/testbeam_connections.csv'
p.sequence = [
    hcal_digi.HcalSingleEndRecProducer(
        coll_name = 'HcalRawDigis',
        rec_coll_name = 'HcalSingleEndRecHits',
        rec_pass_name = '',
    ),
    hcal_digi.HcalDoubleEndRecProducer(
        pass_name = '',
        coll_name = 'HcalSingleEndRecHits',
        rec_coll_name = 'HcalDoubleEndRecHits',
        rec_pass_name = ''
    )
]
