"""Decoding configuration for raw testbeam data                                                                                                                                                                                                                                                                                                                      
                                                                                                                                                                                                                                                                                                                                                                     
Decoding **DOES NOT** attempt                                                                                                                                                                                                                                                                                                       
to align the two halves of the HCal. We assume a local home path of ldmx-sw installation e.g.                                                                                                                                                                                                                                                                                                                  
                                                                                                                                                                                                                                                                                                                                                                     
  LDMX_BASE=/local/cms/user/eichl008/ldmx/                                                                                                                                                                                                                                                                                                                           
                                                                                                                                                                                                                                                                                                                                                                     
so that we can construct the output path correctly.                                                                                                                                                                                                                                                                                                                  
                                                                                                                                                                                                                                                                                                                                                                     
The run number is deduced from the file name.

@author Tom Eichlersmith, University of Minnesota
"""

import argparse, sys

parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}',
    description=__doc__)

parser.add_argument('input_file')
parser.add_argument('--pause',action='store_true')
parser.add_argument('--max_events',type=int)
parser.add_argument('--pedestals',default=None,type=str)

arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('decode')
if arg.max_events is not None :
    p.maxEvents = arg.max_events
p.termLogLevel = 0
p.logFrequency = 1000

import LDMX.Hcal.hgcrocFormat as hcal_format
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
from LDMX.DQM import dqm
import os
from LDMX.Hcal.DetectorMap import HcalDetectorMap
detmap = HcalDetectorMap(f'{os.environ["LDMX_BASE"]}/ldmx-sw/Hcal/data/testbeam_connections.csv')
detmap.want_d2e = True # helps quicken the det -> elec translation                                                                                                                                                                                                                                                                                                   

# extract and deduce parameters from input file name                                                                                                                                                                                                                                                                                                                 
params = os.path.basename(arg.input_file).replace('.root','').split('_')
run = params[params.index("run")+1]
day = params[-2]
time = params[-1]
if 'fpga' in params :
    alignment = 'unaligned'
    pf = params[params.index("fpga")+1]
    provided = f'fpga_{pf}'
    input_names = [ f'Polarfire{pf}Raw' ]
    out_name = input_names[0]+'Digis'
elif 'hcal' in params :
    alignment = 'aligned'
    provided = 'hcal'
    input_names = [ f'Polarfire{pf}Raw' for pf in [0,1] ]
    out_name = 'HcalRawDigis'
else :
    raise KeyError(f'Unable to deduce alignment from {fp}, need either "fpga" or "hcal" in base file name.')

dir_name  = f'{os.environ["LDMX_BASE"]}/testbeam/{alignment}/v2-decoded'
os.makedirs(dir_name, exist_ok=True)
os.makedirs(dir_name+'-ntuple', exist_ok=True)

file_name = f'decoded_{provided}_run_{run}_{day}_{time}'

p.inputFiles = [arg.input_file]
p.outputFiles = [f'{dir_name}/{file_name}.root']
p.histogramFile = f'{dir_name}-ntuple/ntuple_{file_name}.root'

# sequence                                                                                                                                                                                                                                                                                                                                                           
#   1. decode event packet into digi collection                                                                                                                                                                                                                                                                                                                      
#   2. ntuplize digi collection                                                                                                                                                                                                                                                                                                                                      
p.sequence = [
        hcal_format.HcalRawDecoder(
            input_names = input_names,
            output_name = out_name
            ),
        dqm.NtuplizeHgcrocDigiCollection(
            input_name = out_name,
            pedestal_table = arg.pedestals
            )
        ]

if arg.pause :
    p.pause()

