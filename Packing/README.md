# Packing

This ldmx-sw module is focused on interfacing between the raw, binary data coming out of LDMX's DAQ path
and the centralized, hierachical data that is used for the rest of our processing chain.

The hierarchical objects that are most similar to the raw binary data are the "digi" objects.
These objects (especially the HgcrocDigiCollection) directly reference the encoded words that are a part of the raw binary files.

The data coming off of our detector will be streamed and the handling of this data while it is being streamed is done by an "online" set of software.
After the data is streamed (the end point being some sort of file), the data can be imported into "offline" software.
This process is naturally two steps, so I see the software necessary to decode and encode data also to be two steps.

## TestBeam

We now have a procedure for decoding data into the same ROOT file. 
**We do not align the events**, but it makes it much easier to do analyses across subsystems. 
This procedure has two steps due to the complicated nature of TS readout.

### 1. Reformat the raw TS data so that it is grouped into raw event data.
```
ldmx python3 ldmx-sw/TrigScint/util/decode_2fiber_toRAW_fromBin.py --passThrough -i ldmx_captan_out_<run-info>.dat
```
- `--passThrough` is used so that the reformating script does not veto any events. `-a` uses ADC as a veto, `-t` uses TDC and is experimental.
- There is a lot of printout. The error `CID between fiber1 and fiber2 unsynced!` is a known bug in the system and is not affecting the decoding.
- The end of this script prints out how many events it grouped which could be used to know how many events to request in the `decode.py` script.

### 2. Decode raw data from various subsystems into one output ROOT event file and output ntuple file.
The `Packing/decode.py` script that did this step depended on the removed
`HcalRawDecoder` and has been removed. Use an ldmx-sw release before its
removal to decode the 2022 test beam data.
