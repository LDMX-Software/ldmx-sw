# Packing

This ldmx-sw module is focused on interfacing between the raw, binary data coming out of LDMX's different subsystems 
and the centralized, hierachical data that is used for the rest of our processing chain.

The hierarchical objects that are most similar to the raw binary data are the "digi" objects.
These objects (especially the HgcrocDigiCollection) directly reference the encoded words that are a part of the raw binary files.

The data coming off of our detector will be streamed and the handling of this data while it is being streamed is done by an "online" set of software.
After the data is streamed (the end point being some sort of file), the data can be imported into "offline" software.
This process is naturally two steps, so I see the software necessary to decode and encode data also to be two steps.

## The "online" Step: Merge
Merge the various raw files coming off simultaneouslly from the different subsystems into a single file. 
At this time, we also partially unpack the binary files into a ROOT TTree that can be fed into ldmx-sw.
By "partially" I mean we only unpack the binary files in a way that is uniform across all subsystems.
This way, we can isolate any subsystem/chip-specific decoding to the second step.

This step is meant to be isolated from ldmx-sw so hardware close to the detector that does
this merging does not need a full build of ldmx-sw to function.

I have heard front-end people refer to this step as "Event Building".
Step 2 below basically assumes that the product of the "Event Building" step is a ROOT
file with the data organized in the structure described below.

### Raw Data Object
The layout of a raw data file (after the "event building" or "merging" step but before the "unpacking" step)
is slightly hierarchical.

```yaml
ROOT TFile :
  ROOT TTree (LDMX_RawData) :
    TBranch (data_stream0) : std::vector<uint64_t>
    TBranch (data_stream1) : std::vector<uint64_t>
    ... other event data ...
  ROOT TTree (LDMX_RawRun) :
    TBranch (run_number) : int
    ... other run data ...
```

The idea is to have this `RawEventFile` be a stepping-stone to the fully hierarchical `EventFile`.
The majority of the data from the different subsystems is grouped into the various branches of `LDMX_RawData`.
Each "data stream" is given a branch of `std::vector<uint64_t>` to copy its buffer for a single event into.
The name of the branch is how a translator for that buffer is chosen.

## The "offline" step: Unpack
The "offline" software can then handle the complexity of actually decoding the data that has been read off the detector.
This decoding step is done by a single processor in ldmx-sw which dynamically loads a set of "translators" based on the identifying names for each stream of data. 
This also isolates all subsystem-specific decoding into subsystem-specific places.
We can then create the "digi" objects that can be utilized by further reconstruction steps.

This step is meant to be _the_ translation step from "online" data to "offline" data (ldmx-sw).

Each dynamically-loaded translator is available to be matched with a specific branch in the `LDMX_RawData`
tree and can be used to decode the buffer stored in that branch.
The translators are given the full handle to the event bus, allowing them to create multiple event objects from one buffer and/or add information to the event header.

**The creation of an EventHeader and a RunHeader from the buffered data has not been implemented yet.**

## Band Aids
While we are waiting to nail down the specifics of the readout of the various subsystems,
we will need some extra code to insert the expected header information that will surround
the raw files currectly being read off a stand-a-lone chip.

### HexaReformat
Specifically, I have written a short program `hexa-reformat` which converts the files output
by the software I'm using to test an HGC ROC into the form detailed above.
The code for this conversion is in the `HexaReformat` directory and can be compiled independently from ldmx-sw.
