#!/usr/bin/env python3
"""Generate a framed-style fragment input file for the event builder.

Format matched to eventbuilder/src/main.cc fallback parser:
 - uint64 timestamp_raw
 - uint64 subsystem_id
 - size_t payload_size (8 bytes on x86_64)
 - payload bytes
 - optional FragmentTrailer (uint32 checksum)

This script writes several fragments within a 5ms window across multiple subsystems.
"""
import struct
import time

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


def write_fragments(path='fake_fragments.dat'):
    # base timestamp in ns
    base_ns = int(time.time()) * 1000000000
    # create three fragments within 1ms of each other
    fragments = [
        # (offset_ns, subsystem_id, payload)
        (0, 0, b'TRACKER_PAYLOAD'),
        (500000, 1, b'HCAL_PAY'),
        (900000, 2, b'ECAL!')
    ]

    with open(path, 'wb') as f:
        for off, sid, payload in fragments:
            ts = base_ns + off
            # timestamp_raw (uint64)
            f.write(struct.pack('<Q', ts))
            # subsystem_id (uint64)
            f.write(struct.pack('<Q', sid))
            # payload_size as size_t -> 8-byte unsigned on this platform
            psz = len(payload)
            f.write(struct.pack('<Q', psz))
            # payload
            if psz:
                f.write(payload)
            # trailer: uint32 checksum
            csum = crc32_cpp(payload)
            f.write(struct.pack('<I', csum))

    print(f'Wrote fake fragments to {path} (base ts {base_ns})')


if __name__ == '__main__':
    write_fragments()
