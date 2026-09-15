#!/usr/bin/env python3
"""
Script to read and display events from EventBuilder's binary output (events.bin)
"""

import struct
import sys

def read_events(filename, max_events=10):
    """Read and print events from binary file"""
    try:
        with open(filename, 'rb') as f:
            event_count = 0
            while event_count < max_events:
                # Read event header
                header_data = f.read(20)  # uint64_t event_id + uint64_t ts + uint32_t nblocks
                if len(header_data) < 20:
                    break
                
                event_id, timestamp, nblocks = struct.unpack('<QQI', header_data)
                print(f"\n{'='*80}")
                print(f"Event {event_count + 1}:")
                print(f"  Event ID: {event_id}")
                print(f"  Timestamp: {timestamp} ns")
                print(f"  Number of blocks: {nblocks}")
                print(f"  Blocks:")
                
                total_payload = 0
                for block_idx in range(nblocks):
                    # Read block header
                    block_header = f.read(24)  # uint64_t sid + uint64_t bts + uint32_t psz + uint32_t csum
                    if len(block_header) < 24:
                        print("    ERROR: Could not read complete block header")
                        return
                    
                    subsys_id, block_ts, payload_size, checksum = struct.unpack('<QQIi', block_header)
                    
                    # Read block payload
                    payload = f.read(payload_size)
                    if len(payload) < payload_size:
                        print(f"    ERROR: Could not read complete payload for block {block_idx}")
                        return
                    
                    total_payload += payload_size
                    
                    print(f"    [{block_idx}] Subsystem ID: {subsys_id}")
                    print(f"        Timestamp: {block_ts} ns")
                    print(f"        Payload size: {payload_size} bytes")
                    print(f"        Checksum: 0x{checksum:08x}")
                    print(f"        Data (first 64 bytes): {payload[:64].hex()}")
                    if len(payload) > 64:
                        print(f"        ... ({len(payload) - 64} more bytes)")
                
                print(f"  Total payload: {total_payload} bytes")
                event_count += 1
            
            print(f"\n{'='*80}")
            print(f"Successfully read {event_count} events from {filename}")
            
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <events.bin> [max_events]")
        print(f"Example: {sys.argv[0]} events.bin 10")
        sys.exit(1)
    
    filename = sys.argv[1]
    max_events = int(sys.argv[2]) if len(sys.argv) > 2 else 10
    
    read_events(filename, max_events)
