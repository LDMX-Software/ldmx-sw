# Packing

This ldmx-sw module is focused on interfacing between the raw, binary data coming out of LDMX's DAQ path
and the centralized, hierachical data that is used for the rest of our processing chain.

The hierarchical objects that are most similar to the raw binary data are the "digi" objects.
These objects (especially the HgcrocDigiCollection) directly reference the encoded words that are a part of the raw binary files.

The data coming off of our detector will be streamed and the handling of this data while it is being streamed is done by an "online" set of software.
After the data is streamed (the end point being some sort of file), the data can be imported into "offline" software.
This process is naturally two steps, so I see the software necessary to decode and encode data also to be two steps.

## The "online" Step: Merge
Merge the various raw files coming off simultaneouslly from the different subsystems into a single file. 
This is done by a [Rogue](https://github.com/slaclab/rogue) "collector" which then writes the information 
into a binary file format that we define.

I have heard front-end people refer to this step as "Event Building".
Step 2 below basically assumes that the product of the "Event Building" step is a binary
file with the data organized in the structure described below.

This step is meant to be isolated from ldmx-sw so hardware close to the detector that does
this merging does not need a full build of ldmx-sw to function.

### Raw Data File
The layout of a raw data file (after the "event building" or "merging" step but before ldmx-sw) is slightly hierarchical.
Since the structure of this data file is in flux, I have drafted it in a [set of google slides](https://docs.google.com/presentation/d/1bCd3qViZYVYngBMQj1FaEKdDUv0ILHD2l3jWDfYfTBI/edit?usp=sharing).

## The "offline" step: Unpack
The "offline" software can then handle the complexity of actually decoding the data that has been read off the detector.
This decoding step is done by a single processor in ldmx-sw which dynamically loads a set of "translators" based on the identifying names for each stream of data. 
This also isolates all subsystem-specific decoding into subsystem-specific places.
We can then create the "digi" objects that can be utilized by further reconstruction steps.

This step is meant to be _the_ translation step from "online" data to "offline" data (ldmx-sw).

The complexity of decoding necessitates breaking this step into two substeps.

### Re-Splitting
First, we need to re-split the event data into the different subsystems. 
This is done by a central processor which adds each of the subsystem packets into the event bus with an identification name that is determined from the subsystem ID number.
This central processor will also modify the event header and run header to align with what is stored in the raw file.

### Subsystem Decoding
After the re-splitting step, the different subsystems can have their own producers acquire the subsystem packets from the event bus and do the decoding.
