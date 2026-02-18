#!/usr/bin/env python3
import struct

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


def write_event(path='events_out.dat'):
    with open(path, 'wb') as f:
        # Event header: event_id (u64), ts (u64), nblocks (u32)
        event_id = 1
        ts = 1234567890
        blocks = []

        payload1 = b'hello'
        c1 = crc32_cpp(payload1)
        blocks.append((1, ts, payload1, c1))

        payload2 = b'\x00\x01\x02'
        c2 = crc32_cpp(payload2)
        blocks.append((2, ts + 1000, payload2, c2))

        f.write(struct.pack('<Q', event_id))
        f.write(struct.pack('<Q', ts))
        f.write(struct.pack('<I', len(blocks)))

        for sid, bts, payload, csum in blocks:
            f.write(struct.pack('<Q', sid))
            f.write(struct.pack('<Q', bts))
            f.write(struct.pack('<I', len(payload)))
            f.write(struct.pack('<I', csum))
            if payload:
                f.write(payload)


if __name__ == '__main__':
    write_event()
    print('Wrote events_out.dat')
