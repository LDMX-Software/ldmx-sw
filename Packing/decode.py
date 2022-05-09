"""Basic HGCROCv2RawDataFile reformatting configuration"""

import argparse, sys

parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}')

parser.add_argument('output_file')

parser.add_argument('--wr')
parser.add_argument('--ts')
parser.add_argument('--ft41')
parser.add_argument('--ft42')
parser.add_argument('--ft50')
parser.add_argument('--ft51')
parser.add_argument('--pf0')
parser.add_argument('--pf1')
parser.add_argument('--hcal')

parser.add_argument('--max_events',default=100,type=int)
parser.add_argument('--pause',action='store_true')
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack')
p.maxEvents = arg.max_events
p.termLogLevel = 1
p.logFrequency = 1

import LDMX.Hcal.hgcrocFormat as hcal_format
import LDMX.TrigScint.qieFormat as ts_format
from LDMX.DQM import dqm
from LDMX.Packing import rawio

p.outputFiles = [arg.output_file]

# where the ntuplizing tree will go
import os
p.histogramFile = f'{os.path.dirname(arg.output_file)}ntuple_{os.path.basename(arg.output_file)}'

if arg.wr is not None :
    p.sequence.append(
        rawio.WRRawDecoder(
            raw_file = arg.wr,
            output_name = 'WRRaw')
        )

if arg.ft41 is not None :
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            raw_file = arg.ft41,
            name = 'ft41',
            output_name = 'FiberTracker41Raw')
        )

if arg.ft42 is not None :
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            raw_file = arg.ft42,
            name = 'ft42',
            output_name = 'FiberTracker42Raw')
        )

if arg.ft50 is not None :
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            raw_file = arg.ft50,
            name = 'ft50',
            output_name = 'FiberTracker50Raw')
        )

if arg.ft51 is not None :
    p.sequence.append(
        rawio.FiberTrackerRawDecoder(
            raw_file = arg.ft51,
            name = 'ft51',
            output_name = 'FiberTracker51Raw')
        )

if arg.pf0 is not None :
    p.inputFiles = [arg.pf0]
    p.sequence.extend([
        hcal_format.HcalRawDecoder(
            input_names = ['Polarfire0Raw'],
            input_pass = 'raw',
            output_name = 'PF0Digis'
            ),
        dqm.NtuplizeHgcrocDigiCollection(
            input_name = 'PF0Digis',
            name = 'pf0'
            )
        ])

if arg.pf1 is not None :
    p.inputFiles = [arg.pf1]
    p.sequence.extend([
        hcal_format.HcalRawDecoder(
            input_names = ['Polarfire1Raw'],
            input_pass = 'raw',
            output_name = 'PF1Digis'
            ),
        dqm.NtuplizeHgcrocDigiCollection(
            input_name = 'PF1Digis',
            name = 'pf1'
            )
        ])

if arg.hcal is not None :
    p.inputFiles = [arg.hcal]
    p.sequence.extend([
        hcal_format.HcalRawDecoder(
            input_names = ['Polarfire0Raw','Polarfire1Raw'],
            input_pass = 'raw',
            output_name = 'HcalDigis'
            ),
        dqm.NtuplizeHgcrocDigiCollection(
            input_name = 'HcalDigis',
            name = 'hcal'
            )
        ])

if arg.ts is not None :
    n_channels = 16
    n_timesamples = 30
    header_len = 4+4+4+3+1
    qie_decoder = ts_format.QIEDecoder.up(
            os.environ['LDMX_BASE']+
            '/ldmx-sw/TrigScint/data/'+
            'channelMap_LYSOback_plasticFront_12-to-16channels_rotated180.txt')
    qie_decoder.number_channels = n_channels
    qie_decoder.number_time_samples = n_timesamples
    qie_decoder.is_real_data = True
    p.sequence.extend([
        rawio.SingleSubsystemUnpacker(
            raw_file = arg.ts,
            output_name = 'QIEstreamUp',
            detector_name = 'ldmx-hcal-prototype-v1.0',
            num_bytes_per_event = 2*n_channels*n_timesamples + header_len
            ),
         qie_decoder,
        dqm.NtuplizeTrigScintQIEDigis(
            input_name = 'decodedQIEUp'
            )
        ])

if arg.pause :
    p.pause()
