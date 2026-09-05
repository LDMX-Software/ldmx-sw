# EventBuilder

EventBuilder reads raw detector data from `.dat` files and assembles complete physics events by grouping detector subsystem fragments based on timestamp coherence.

## Overview

The EventBuilder operates as a Framework producer that:
1. **Reads binary frames** from a `.dat` file using the Reader class for incremental streaming
2. **Parses frame headers** (RogueFrameHeader for StreamWriter framing)
3. **Extracts fragment data** in either RoR (LDMXRoRHeader) or packing subsystem format
4. **Buffers fragments** by timestamp in a FragmentBuffer
5. **Assembles events** when fragments from ≥3 different subsystems arrive within a 5ms coherence window
6. **Outputs events** to:
   - ROOT file with event-by-event subsystem data (Framework output)
   - Binary file (`events.bin`) for offline analysis
   - CSV summary (`event_summaries.txt`) for quick validation

## Architecture

### Core Classes

**EventBuilder** (`EventBuilder.hh`)
- Main Producer class that drives event assembly
- `configure()`: Opens the .dat file with Reader
- `produce()`: Main event loop - reads frames, parses fragments, buffers them, and assembles events

**FragmentBuffer** (`FragmentBuffer.hh`)
- Maintains a map of fragments organized by timestamp
- `add_fragment()`: Stores incoming fragments, sets reference time on first fragment
- `try_build_event()`: Collects all fragments within `[ref_time ± coherence_window]`, requires ≥3 unique subsystems
- Returns false if completeness check fails (incomplete event)

**DataFragment** (`Fragment.hh`)
- Header: subsystem_id, timestamp, contributor
- Trailer: CRC checksum
- Payload: Raw byte data from device

**PhysicsEventData** (`Event/PhysicsEventData.hh`)
- Event-level data structure for binary output
- Contains: event_id, timestamp, blocks (one per subsystem), systems_readout list

**GenericDataBlock** (`Event/GenericDataBlock.hh`)
- Per-subsystem data in an event
- Contains: subsystem_id, timestamp_ns, data (uint8_t vector), checksum

**EventSummary** (`EventSummary.hh`)
- CSV row for quick event validation
- Contains: event_id, timestamp_ns, # subsystems, subsystem_ids, total_payload_size, error_flags

### Data Flow

```
.dat file (binary frames)
    ↓
RogueFrameHeader (frame demux, skip non-data channels)
    ↓
LDMXRoRHeader or SubsystemPacket (format detection & parsing)
    ↓
DataFragment (structured fragment)
    ↓
FragmentBuffer (timestamp-based buffering)
    ↓
try_build_event (coherence window assembly, subsystem completeness check)
    ↓
PhysicsEventData (event payload) → Binary file + CSV summary
    ↓
Framework::Event → Root file (subsystem branches)
```

## Usage

### Python Configuration

```python
from LDMX.Framework import ldmxcfg
from LDMX.EventBuilder import eventbuilder
import sys

p = ldmxcfg.Process('unpack')
p.run = 1
p.max_events = 100
p.verbose_parse = True

p.sequence = [
    eventbuilder.from_dat_file(sys.argv[1])  # Input .dat file
]

p.output_files = [sys.argv[2]]  # Output .root file
p.pause()
```

Run with:
```bash
python run_eventbuilder.py input.dat output.root
```

### Configuration Parameters

- **dat_file**: Path to input binary file (or set EVENTBUILDER_INPUT env var)
- **output_name**: Name of object on event bus (default: "PhysicsEventData")
- **verbose_parse**: Enable debug logging (default: false)

### Output Files

1. **output.root** (Framework output)
   - One TTree with event entries
   - Branches: TDAQ_Trigger, TS_DAQ, TS_Trigger, Tracker_DAQ, ECAL_DAQ, ECAL_Trigger, HCAL_DAQ, HCAL_Trigger
   - Each branch contains `std::vector<uint8_t>` for that subsystem's raw data
   - All subsystems present in an event are grouped in the same tree entry

2. **events.bin** (Binary format)
   - Compact binary stream of assembled events
   - Format: `[event_id: u64][timestamp: u64][nblocks: u32][blocks...]`
   - Each block: `[subsys_id: u64][ts: u64][size: u32][checksum: u32][data: bytes]`

3. **event_summaries.txt** (CSV)
   - One line per event
   - Fields: event_id, timestamp_ns, nsystems, system_ids (semicolon-separated), payload_size, error_flags
   - Quick validation without parsing binary

## Subsystem ID Mapping

Events can contain data from these subsystems (IDs 1-9):

| ID | Name | Description |
|---|---|---|
| 1 | TDAQ_Trigger | TDAQ trigger data |
| 2 | TS_DAQ | Target spectrometer DAQ |
| 3 | TS_Trigger | Target spectrometer trigger |
| 4 | Tracker_DAQ | Tracker detector |
| 5 | ECAL_DAQ | Electromagnetic calorimeter |
| 6 | ECAL_Trigger | ECAL trigger |
| 7 | HCAL_DAQ | Hadronic calorimeter |
| 8 | HCAL_Trigger | HCAL trigger |
| 9 | Generic | Other/unknown subsystem |

## Validation

### Method 1: Check CSV Summary

```bash
head event_summaries.txt
```

Example output:
```
1,51489468331918879,4,1;2;4;5,47140,0
2,51489468448398879,4,1;2;4;5,47120,0
```

Event 1: 4 subsystems (IDs 1,2,4,5), timestamp 51489468331918879 ns, 47140 bytes total

### Method 2: Read Binary File

```bash
python EventBuilder/scripts/read_events.py events.bin
```

Example output:
```
================================================================================
Event 1:
  Event ID: 1
  Timestamp: 51489468331918879 ns
  Number of blocks: 4
  Blocks:
    [0] Subsystem ID: 1
        Timestamp: 51489468331918879 ns
        Payload size: 32 bytes
        Data (hex): 06990d66edb60000...
    [1] Subsystem ID: 2
        Timestamp: 51489468331918879 ns
        Payload size: 6736 bytes
        ...
```

### Method 3: Inspect ROOT File

```bash
root output.root
root [0] Events->Draw("Entries$(ECAL_DAQ)")
root [1] Events->Print()
```

Check:
- Number of entries matches `event_summaries.txt`
- Branches present match expected subsystems
- Data sizes are non-zero

## Coherence Window & Event Assembly

**Coherence Window**: 5 milliseconds (5,000,000 nanoseconds)

When a fragment arrives:
1. If buffer is empty, set reference time = fragment timestamp
2. If fragment is within ±5ms of reference time, add to current event batch
3. If fragment is **outside** the window, finalize current event and start new batch with this fragment

**Completeness Check**: Event is complete when it contains ≥3 unique subsystems

This ensures:
- Fragments from the same physics event (arriving within detector timing) group together
- Fragments from different beam spills (separated by >5ms) don't merge
- Partial events (missing subsystems) are rejected until they timeout or new fragment outside window triggers assembly


## Development Notes

- **Reader class**: From Packing library, enables incremental binary streaming without loading entire file
- **RogueFrameHeader**: StreamWriter framing from Rogue DAQ system
- **LDMXRoRHeader**: Custom header format used by LDMX RoR protocol  
- **SubsystemPacket**: Alternative packet format from Packing/RawDataFile
- **Reference time tracking**: FragmentBuffer maintains window center for robust multi-subsystem grouping
- **No ROOT dictionaries**: Event data is stored as native `std::vector<uint8_t>` which ROOT serializes natively
