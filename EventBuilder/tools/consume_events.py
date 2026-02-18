#!/usr/bin/env python3
"""Simple consumer to read `events_out.dat` written by the event builder.

Usage: ./consume_events.py [path]
"""
import sys
import struct
# Port of the C++ CRC32 implemented in include/Fragment.hh
def crc32_cpp(data: bytes) -> int:
    crc = 0xFFFFFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xEDB88320
            else:
                crc >>= 1
    return (~crc) & 0xFFFFFFFF


def read_u64(f):
    b = f.read(8)
    if len(b) < 8:
        raise EOFError
    return struct.unpack('<Q', b)[0]


def read_u32(f):
    b = f.read(4)
    if len(b) < 4:
        raise EOFError
    return struct.unpack('<I', b)[0]


def consume(path='events_out.dat'):
    with open(path, 'rb') as f:
        idx = 0
        try:
            while True:
                event_id = read_u64(f)
                ts = read_u64(f)
                nblocks = read_u32(f)
                print(f"Event #{event_id} ts={ts} ns blocks={nblocks}")
                for i in range(nblocks):
                    sid = read_u64(f)
                    bts = read_u64(f)
                    psz = read_u32(f)
                    csum = read_u32(f)
                    payload = f.read(psz) if psz else b''
                    calc = crc32_cpp(payload)
                    ok = (csum == 0 or csum == calc)
                    print(f"  Block {i}: subsystem={sid} block_ts={bts} size={psz} checksum=0x{csum:08x} ok={ok}")
                idx += 1
        except EOFError:
            pass


def main(argv):
    path = argv[1] if len(argv) > 1 else 'events_out.dat'
    try:
        consume(path)
    except FileNotFoundError:
        print(f"File not found: {path}")


if __name__ == '__main__':
    main(sys.argv)
