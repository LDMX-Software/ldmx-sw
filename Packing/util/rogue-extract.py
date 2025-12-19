#!/usr/bin/env python3
import pyrogue.utilities.fileio
import sys
import numpy as np
import argparse
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description="just hexdump the Hcal/Ecal data without attempting to decode")
    parser.add_argument('input', help='input file to Hcal/Ecal data from')
    parser.add_argument('--nevent', '-n', help='maximum number of events to decode', type=int)
    parser.add_argument('--contrib', '-c', choices=['ecal','hcal'], help='contributor to focus on, without this dump all data from subsystem 5 which could be both Ecal and Hcal')
    parser.add_argument('--subsystem', '-s', choices=[5,2], help='contributor to focus on, without this dump all data from subsystem 5 which could be both Ecal and Hcal')
    args = parser.parse_args()

    contrib_id = None
    if args.contrib is None:
        contrib_id = 0 # undefined
    elif args.contrib == 'hcal':
        contrib_id = 20
    elif args.contrib == 'ecal':
        contrib_id = 40
    elif args.contrib == 'ts':
        contrib_id = 1

    subsystem_id = None
    if args.subsystem is None:
        subsystem_id = 5 # assume hcal/ecal
    else :
        subsystem_id = args.subsystem
        
    count = 0
    # do NOT provide configChan = 255 because it does not provide a Loader to yaml.load
    # which fails in newer versions of Python :tada:
    with pyrogue.utilities.fileio.FileReader(files=sys.argv[1]) as fd:
        for header, data in fd.records():
            if data[-1] == 0x0a or header.channel != 0:
                # magic byte signaling config packet
                continue

            # a byte from the Rogue header signals the subsystem
            # from slaclab/ldmx-firmware/common/tdaq/python/ldmx_tdaq/_Constants.py
            subsys = data[1]
            # both Hcal and Ecal are using the EcalSubsystem in RunControl
            # meaning they are both using the ID 5 right now
            if (subsys != subsystem_id):
                continue

            # we are planning to use the contributor_id to separate Ecal and Hcal
            contrib = data[2]
            if contrib_id != 0 and contrib != contrib_id:
                continue

            # check on event limit
            if args.nevent is not None and count >= args.nevent:
                break

            # need to convert the data into a std::vector<uint32_t>
            # data here is a np.ndarray('int8') so we re-view
            # it in our words and skip the first four words which
            # contain the Rogue-inserted EventHeader
            words = data.view('uint32')
            print('--------')
            for w in words:
                print(f'{w:08x}')

            count += 1
    
if __name__ == '__main__':
    main()
