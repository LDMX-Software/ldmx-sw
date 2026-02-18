#!/usr/bin/env python3
import struct

# Build a minimal RoR-style word-aligned frame.
# We'll craft a little-endian example where low byte is 0xA5 sentinel.
# Place subsystem in byte 1 (bits 8..15) or try others by changing subsys_shift.

# Choose subsystem ID to test
subsys = 0x05  # ECAL_DAQ_SUBSYSTEM

# Compose w0: low byte sentinel 0xA5, subsystem in byte 1
# little-endian word: byte0 = sentinel, byte1 = subsys, byte2/3 arbitrary
w0 = (subsys << 8) | 0xA5
w1 = 0
w2 = 0
# put timestamp in w3 (seconds)
w3 = 1600000000
words = [w0, w1, w2, w3]

with open('test_frame.dat','wb') as f:
    for w in words:
        f.write(struct.pack('<I', w))

print('wrote test_frame.dat with subsystem 0x{:02X}'.format(subsys))
