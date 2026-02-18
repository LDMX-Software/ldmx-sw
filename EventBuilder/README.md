# Introduction

# Purpose

A simple starting point for the LDMX Event builder

# Compile

To run with cmake:

```
mkdir build
cd build
cmake ..
cd ..
cmake --build build
```
 
## Usage (file-stream mode)

By default the event builder reads `Run_061_20251211_145655.dat` in the project root.
You can override the input file by setting the `EVENTBUILDER_INPUT` environment variable:

```bash
export EVENTBUILDER_INPUT=/path/to/Run_061_20251211_145655.dat
./bin/event_builder
```

To enable pflib-based RoR decoding, build with `-DUSE_PFLIB=ON` and provide pflib include/link paths in `CMakeLists.txt`.

# Concept

Summary: The journey of a real-world event:

* Detector Signal: A particle interaction creates analog signals in the detector.
* Digitization: Specialized electronics convert the analog signals into digital data.
* Serialization: On-detector firmware and ASICs format the digital data into a standardized binary protocol, adding timestamps and event IDs. This creates the serialized "data fragment."
* Transmission: The fragments are sent over high-speed networks to a central data acquisition system.
* Buffering and Synchronization: The central system receives fragments from all sub-detectors, buffers them, and waits for a complete set belonging to the same event ID.
* Deserialization: The raw byte fragments are parsed and converted back into structured C++ objects.
* Event Building: The structured data from all sub-detectors is aggregated into a single, cohesive "event" object.
* Analysis: The final event object is then ready for further processing and analysis.

# This code

## `main.cc`:

This file demonstrates a complete, simulated workflow for a particle physics event builder. It orchestrates the process from generating simulated raw detector data to producing a final, combined event object ready for analysis.

The current setup has two options:

* Real data via TCP bridge
* From file input reading existing data .dat from disk

## `EventBuilder.hh`:

Defines a generic EventBuilder class that encapsulates the construction of a `GenericEvent` object. It takes the deserialized payload and other contributing information and combines them to produce a complete, coherent event object.

`GenericEvent` constructed from:

* `Payload`: A generic type representing the data collected from the various detector subsystems for a single event. In the main.cpp example, this is PhysicsEventData.
* `ContributorType`: A type representing the source or category of the event data.


The `EventBuilder` then takes this deserialized Payload and assembles it into the final `GenericEvent` object, bundling the event data with its metadata (ContributorType). This makes the data ready for downstream processing, such as analysis or filtering by a High-Level Trigger.

## `Contributor.hh`

`Contributor.hh`, defines an abstract base class named `Contributor`. This class is a key part of the program's object-oriented design, establishing a common interface for any object that can be a source of event data.

## `DataAggregator.hh`

The `DataAggregator.hh` header file defines a simple class, `DataAggregator`, which is designed to represent a source of event data. It inherits from a `Contributor` class, making it a concrete implementation of that interface.


## `PhysicsEventData.hh`

`PhysicsEventData.hh` defines a C++ struct that serves as the central data payload for a fully assembled particle physics event. It combines information collected from multiple subsystems into a single, cohesive data structure, ready for further processing and analysis.

In the `main.cc` process the `assemble_payload` function uses the `PhysicsEventData` struct to collect raw `DataFragment`s (payloads remain encoded) so downstream components can decode them as needed.

## `FragmentBuffer.hh`

The `FragmentBuffer.hh` header defines a class critical for the event building process, acting as a temporary storage and management area for raw data fragments from detector subsystems before they are combined into a complete event. It is responsible for storing fragments, grouping them by `event_id`, checking for event completeness, supplying fragments for processing, and managing memory and potential timeouts. Core components likely include data structures (like `std::map` or `std::unordered_map`) for fragment storage, methods for adding and retrieving fragments, logic for detecting complete events (like `try_build_event`), and mechanisms for concurrency management such as mutexes and conditional variables.

## `Fragment.hh`

The `Fragment.hh` header file defines the fundamental data structures used in the particle physics event building process. It formalizes what a "data fragment" is by breaking it into a structured header containing metadata and a raw payload of data.

The `FragmentHeader` struct contains all the essential metadata needed to manage a data fragment.

    * `timestamp`: The time recorded by the Level-1 (L1) trigger when the particle collision was accepted. This timestamp is vital for correlating fragments, especially if they arrive out of order.
    * `event_id`: A unique identifier assigned by the trigger system to the specific collision. The event builder uses this ID to group fragments that belong to the same event.
    * `contributor_id`: An identifier specifying the source of the data, as defined by the ContributorId enum.
    subsystem_id: The identifier for the detector subsystem, as defined by the SubsystemId enum.
    * `data_size`: The size, in bytes, of the raw data payload. This is important for networking and memory management, as it tells the receiving system how much data to expect for this fragment.

The `DataFragment` struct combines the metadata and the raw data payload into a single, logical unit.

    * `header`: The `FragmentHeader` instance associated with this fragment.
    * `payload`: A `std::vector<char>` containing the raw byte data from the detector's readout electronics. This data is in a serialized format, meaning it's packed as a stream of bytes and needs to be deserialized later by the `assemble_payload` function.

# The Builder

On a second thread the event building takes place. Three values are defined:

```
const long long coherence_window_ns = 5529262;
const long long latency_delay_ns = 100;
const int min_subsystems_for_event = 3;
```

The `coherence_window_ns` describes the event window. The `latency_delay_ns` is an estimate of system lag and the `min_subsystems_for_event` is the expected number of systems contributing to the event.

Initially the builder asks the function `has_expired_fragments(reference_time, coherence_window_ns)` if there is a valid event fragment window.

## Event grouping and assembly (from `main.cc`)

This project implements fragment parsing and timestamp-based grouping in `main.cc`.

- **Parsing fragments:** `main.cc` first tries a word-aligned RoR-style parse (see `parse_ror_header`) to extract a timestamp and subsystem id from each frame; if that fails it falls back to a framed reader that reads timestamp, subsystem id and payload size from the file. See [eventbuilder/src/main.cc](eventbuilder/src/main.cc#L30) and the input paths where fragments are created and added via `event_buffer.add_fragment(...)` ([eventbuilder/src/main.cc](eventbuilder/src/main.cc#L388), [eventbuilder/src/main.cc](eventbuilder/src/main.cc#L499)).

- **Timestamps normalized to nanoseconds:** `parse_ror_header` normalizes timestamps (seconds or ns) into nanoseconds; the merger and summaries operate in ns. See [eventbuilder/src/main.cc](eventbuilder/src/main.cc#L30).

- **Merger thread / coherence window:** A dedicated thread runs `eventMergerThread(FragmentBuffer&)` which repeatedly calls `try_build_event(current_time, coherence_window_ns, ...)` to collect fragments whose timestamps fall within a coherence window (5 ms in the current code). See [eventbuilder/src/main.cc](eventbuilder/src/main.cc#L242) and the `try_build_event` calls ([eventbuilder/src/main.cc](eventbuilder/src/main.cc#L251), [eventbuilder/src/main.cc](eventbuilder/src/main.cc#L284)).

- **Forced/partial assembly on timeout:** If fragments have waited longer than the coherence window, the merger uses `has_expired_fragments(...)` and forces a partial assembly (`try_build_event(..., true)`), producing a summary marked as forced/partial so fragments do not wait indefinitely.

- **Assembling payloads:** Once fragments for an event are returned, `assemble_payload(...)` converts the list of `DataFragment`s into a `PhysicsEventData` object that keeps subsystem payloads encoded (raw `DataFragment`s). The function uses the first fragment's timestamp as the event timestamp and records the subsystem ids present. See [eventbuilder/src/main.cc](eventbuilder/src/main.cc#L158).

- **Summary records:** After assembly the merger increments the global `event_id`, writes an `EventSummary` (event id, timestamp, unique subsystem ids, payload size, and error flags) and appends it to `event_summaries.txt`.

For the exact grouping algorithm (how `FragmentBuffer::try_build_event` buckets timestamps, selects the reference timestamp, and decides completeness), see the `FragmentBuffer` implementation (open `eventbuilder/include/FragmentBuffer.hh` and the corresponding source file). If you want, I can add a short section with direct quotes from `FragmentBuffer` explaining the precise behavior.

### FragmentBuffer: precise grouping behavior

The `FragmentBuffer` stores incoming `DataFragment`s in a timestamp-keyed `std::map<Timestamp, std::vector<DataFragment>>` and provides two key operations that the merger thread uses:

- `add_fragment(DataFragment&&)`: appends a fragment into the map bucket keyed by `fragment.header.timestamp` (protected by a mutex).
- `has_expired_fragments(reference_time, coherence_window_ns)`: returns true when the oldest timestamp in the buffer is older than `reference_time - coherence_window_ns` (used to decide when to force assembly of stale fragments).
- `try_build_event(reference_time, coherence_window_ns, built_fragments, force_assemble=false)`: attempts to collect fragments whose timestamps fall within `[window_ref_time - coherence_window_ns, window_ref_time + coherence_window_ns]` where `window_ref_time` is `reference_time` in normal mode or the oldest fragment's timestamp when `force_assemble==true`.

Behavior details:

- The buffer selects a contiguous set of timestamp keys using `lower_bound(window_ref_time - coherence_window_ns)` and `upper_bound(window_ref_time + coherence_window_ns)` and gathers all fragments from those timestamp buckets.
- For normal (non-forced) assembly the function checks completeness by counting unique subsystem IDs found in the window; the current code requires at least 3 unique subsystems before declaring an event complete and returning the assembled fragments. If that check fails, `try_build_event` returns `false` and leaves the fragments in the buffer.
- When `force_assemble==true` (triggered after `has_expired_fragments` indicates a timeout), the function skips the completeness check and moves all fragments in the selected window into the `built_fragments` output, erasing them from the buffer; the merger marks such events as forced/partial in the `EventSummary`.

See the implementation at [eventbuilder/include/FragmentBuffer.hh](eventbuilder/include/FragmentBuffer.hh#L1) for the exact code paths used by `main.cc`'s merger thread.

## Downstream binary event format

When events are assembled the builder writes a compact binary stream (`events_out.dat` by default) intended for downstream DAQ consumers. The format is intentionally generic and carries only small per-block headers + raw encoded payloads so different subsystem packet formats can be handled without changing the event builder.

Top-level event record (all integers are native little-endian on typical x86):

- `uint64_t event_id` — monotonic event counter assigned by the merger
- `uint64_t timestamp_ns` — event timestamp in nanoseconds
- `uint32_t nblocks` — number of generic blocks in this event

Per-block header repeated `nblocks` times:

- `uint64_t subsystem_id` — subsystem identifier (0=Tracker, 1=Hcal, 2=Ecal, 3=TS, 4=LYSO, 255=Unknown)
- `uint64_t block_timestamp_ns` — original fragment timestamp (ns)
- `uint32_t payload_size` — size in bytes of the following payload
- `uint32_t checksum` — trailer checksum if available (otherwise CRC32 of payload)
- `payload_size` bytes — raw, encoded payload bytes exactly as received

Notes:

- Consumers should read the exact number of bytes specified by `payload_size` for each block and treat the payload as an opaque byte array until they know the subsystem's format.
- The CRC32 checksum is computed using the same function defined in `include/Fragment.hh` when a trailer checksum is not present.
- The builder appends events to the file in binary append mode. Each event is written atomically within a single file append operation protected by a mutex to avoid interleaving when running multi-threaded.

Example (C++) reader sketch:

```cpp
std::ifstream ifs("events_out.dat", std::ios::binary);
while (ifs) {
    uint64_t event_id, ts; uint32_t nblocks;
    ifs.read(reinterpret_cast<char*>(&event_id), sizeof(event_id));
    ifs.read(reinterpret_cast<char*>(&ts), sizeof(ts));
    ifs.read(reinterpret_cast<char*>(&nblocks), sizeof(nblocks));
    for (uint32_t i=0;i<nblocks;++i) {
        uint64_t sid, bts; uint32_t psz, csum; 
        ifs.read(reinterpret_cast<char*>(&sid), sizeof(sid));
        ifs.read(reinterpret_cast<char*>(&bts), sizeof(bts));
        ifs.read(reinterpret_cast<char*>(&psz), sizeof(psz));
        ifs.read(reinterpret_cast<char*>(&csum), sizeof(csum));
        std::vector<char> payload(psz);
        if (psz) ifs.read(payload.data(), psz);
        // handle block
    }
}
```

**Tests**

- **Quick consumer (Python):** Run the consumer against an existing `events_out.dat` file.

    ```bash
    ./tools/consume_events.py events_out.dat
    ```

- **Synthetic end-to-end test (recommended):**

    1. Generate framed fragment input (fallback parser format):

         ```bash
         python3 tools/generate_fake_fragments.py   # writes fake_fragments.dat
         ```

    2. (Optional) Generate a synthetic `events_out.dat` directly for quick consumer checks:

         ```bash
         python3 tools/generate_fake_events.py     # writes events_out.dat
         ./tools/consume_events.py events_out.dat
         ```

    3. Run the real event builder (if you have `bin/event_builder`) on the generated fragments:

         ```bash
         EVENTBUILDER_INPUT=fake_fragments.dat ./bin/event_builder
         # builder appends events to events_out.dat
         ./tools/consume_events.py events_out.dat
         ```

    Notes:
    - The builder reads the input file path from `EVENTBUILDER_INPUT` when set.
    - If the binary crashes, see the Debugging tips below.

- **Pytest integration:** The repository includes `tests/test_events.py` which will run the builder (if present) and the consumer. To run it:

    ```bash
    pytest -q tests/test_events.py
    ```

    The pytest test will skip gracefully if `bin/event_builder` or a valid input file are missing. Use `EVENTBUILDER_INPUT` to point the test at a specific `.dat` file.

**Dependencies & Debugging tips**

- Required: `python3` (for the generator and consumer). Optional but recommended: `pytest`.
- If the binary segfaults, collect these artifacts and share them for analysis:

    ```bash
    # enable core dumps
    ulimit -c unlimited
    EVENTBUILDER_INPUT=fake_fragments.dat ./bin/event_builder || true

    # run with strace
    strace -f -o /tmp/eb_strace.log EVENTBUILDER_INPUT=fake_fragments.dat ./bin/event_builder || true
    tail -n 200 /tmp/eb_strace.log

    # run under valgrind (if installed)
    valgrind --tool=memcheck --leak-check=full EVENTBUILDER_INPUT=fake_fragments.dat ./bin/event_builder 2>/tmp/eb_valgrind.txt
    sed -n '1,200p' /tmp/eb_valgrind.txt
    ```

If you want, run the above and paste the first ~200 lines of the logs here and I will help diagnose the crash.
