#!/usr/bin/env python3
import pyrogue.utilities.fileio
import sys
import numpy as np
import argparse
from pathlib import Path


# helper class to swap endianness of TS data
import struct

def swap_endianness(num):
    # Pack the number as a 32-bit integer in network byte order
    packed_num = struct.pack('>I', num)
    # Unpack the number as a 32-bit integer in little-endian byte order
    unpacked_num = struct.unpack('<I', packed_num)[0]
    return unpacked_num


def main():
    parser = argparse.ArgumentParser(description="just hexdump the Hcal/Ecal data without attempting to decode")
    parser.add_argument('input', help='input file to extract raw data from')
    parser.add_argument('--output','-o',  help='output file to write extracted data to')
    parser.add_argument('--debug', '-d', action='store_true', help='enable debug mode i.e. print some stuff to screen')
    parser.add_argument('--nevent', '-n', help='maximum number of events to decode', type=int)
    parser.add_argument('--contrib', '-c', choices=['ecal','hcal', 'ts'], help='contributor to focus on, without this dump all data from subsystem 5 which could be both Ecal and Hcal')
    parser.add_argument('--subsystem', '-s', choices=['5','2'], help='contributor to focus on, without this dump all data from subsystem 5 which could be both Ecal and Hcal')
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
        print(f'Got contributor: {args.contrib}')

    subsystem_id = None
    if args.subsystem is None:
        subsystem_id = 5 # assume hcal/ecal
    else :
        subsystem_id = int(args.subsystem)

    if args.output :
        output_file_name = args.output
    else :
        output_file_name = sys.argv[1].replace('.dat','_parsed.dat')

    outfile = open(output_file_name, "wb")

       
    count = 0
    pastConfig = False
    rightSubsyst = False
    # do NOT provide configChan = 255 because it does not provide a Loader to yaml.load
    # which fails in newer versions of Python :tada:
    with pyrogue.utilities.fileio.FileReader(files=sys.argv[1]) as fd:
        for header, data in fd.records():
            if data[-1] == 0x0a or header.channel != 0:
                # magic byte signaling config packet
                continue
            if not pastConfig:
                print('got past config packet')
                pastConfig = True 
            # a byte from the Rogue header signals the subsystem
            # from slaclab/ldmx-firmware/common/tdaq/python/ldmx_tdaq/_Constants.py
            subsys = data[1]
            # both Hcal and Ecal are using the EcalSubsystem in RunControl
            # meaning they are both using the ID 5 right now
            if (subsys != subsystem_id):
                continue
            if not rightSubsyst :
                print(f'got to the right subsystem: {subsys}')
                rightSubsyst = True
            # we are planning to use the contributor_id to separate Ecal and Hcal
            contrib = data[2]
            if contrib_id != 0 and contrib != contrib_id:
                continue
            if args.debug :
                print(f'got to the right contributor ID: {contrib_id}')

            # check on event limit
            if args.nevent is not None and count >= args.nevent:
                break

            # need to convert the data into a std::vector<uint32_t>
            # data here is a np.ndarray('int8') so we re-view
            # it in our words and skip the first four words which
            # contain the Rogue-inserted EventHeader
            words = data.view('uint32')
            if args.debug :
                print('--------')
            for w in words:
                if args.debug :
                    print(f'{w:08x}')
                outfile.write(struct.pack('I',w))

            count += 1
            # printout at every tenth of the process
            if args.nevent is not None and count%(args.nevent/10)==0 :
                print(f'At {count} events out of {args.nevent}')
    outfile.close()
    
if __name__ == '__main__':
    main()
