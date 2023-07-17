# Example Configuration Files

## Test-Beam reconstruction

### Before you start:
- Verify that you are working with the latest version of the `Hcal/` submodule.
```
cd ldmx # the parent directory of ldmx-sw
cd ldmx-sw/Hcal
git switch trunk
# rebuild ldmx-sw
cd ../build/
ldmx make install -j 4
```
- Verify that you have the [conditions-data](https://github.com/LDMX-Software/conditions-data) repository installed 
```
cd ldmx # the parent directory of ldmx-sw
git clone git@github.com:ldmx-software/conditions-data
cd ldmx-sw
git checkout trunk
git pull
git submodule update
# rebuild ldmx-sw
cd ../build/
ldmx make install -j 4
```

### For simulation:
- Use simulated events with prototype geometry ```detector='ldmx-hcal-prototype-v2.0'```.
- Run reconstruction on prototype simulation:
```
ldmx fire tb_sim.py hcal_XXX_simevents.root
```

### For data:

- Run reconstruction on prototype simulation:
```
ldmx fire tb_reco.py hcal_run_XXX_decoded.root
```

In this configuration file `HcalSingleEndRecProducer` is run by default. A `DoubleEndRecProducer` will be available too.

#### Decoded inputs
A set of decoded April-2022 TB data is available on the SLAC cluster:
```
/sdf/group/ldmx/data/hcal-tb
```
The directory holds the test beam data from the HCal prototype subsystem.

Since event alignment is a tricky business (and sometimes is broken),
the data has two copies: one with event alignment and one without.
Please look at the README in that directory for more details.

The following is the table of contents of this folder:
```
./
|-- unaligned/ : unification of the two halves of the HCal was not attempted
    |-- reformat/ : unpacking of raw data into event objects, no decoding
    |-- decoded/  : products decoded into HgcrocDigiCollection objects
    |-- decoded-ntuples/ : HgcrocDigiCollection objects ntuplized for easier analysis
    |-- fail-decode.list : list of files that failed the decoding step
    |-- fail-reformat.list : list of files that failed the reformat/unpack step
|-- aligned/ : two polarfires of the HCal were aligned based on timestamps
    |-- reformat/ : simple unpacking and alignemnt, no decoding
    |-- decoded/  : decoded into HgcrocDigiCollection objects
    |-- decoded-ntuples/ : HgcrocDigiCollection objects ntuplized for easier analysis
|-- reformat_cfg.py : configuration script for `ldmx reformat ` used for unpacking and alignment
|-- decode_cfg.py   : configuration script for `ldmx fire ` used for decoding and ntuplization
````

#### Re-doing data reformatting and decoding
Event building was not done online, so it needs to be done on raw TB data files (if not using the existing set of decoded test-beam data). 

A copy of raw April-2022 TB data is available on the SLAC cluster:
```
/sdf/group/ldmx/CERN-TB-DATA/ldmx/testbeam/data/pf_external/
```

- Reformat: 
  - [Build and install reformat](https://github.com/LDMX-Software/ldmx-tb-online/blob/main/reformat/README.md#Building)
  - An [example config: hcal_cfg.py](https://github.com/LDMX-Software/ldmx-tb-online/blob/main/reformat/TestBeam/hcal_cfg.py) exists. A common run example would be:
  ```
  ldmx reformat TestBeam/hcal_cfg.py \
  --pf0 path/to/fpga_0_run_XXX.raw \
  --pf1 path/to/fpga_1_run_XXX.raw \
  --output_filename hcal_run_XXX_reformat.root
  ```
- Decode:
  - One can use the decoding producer in ldmx-sw. 
  - A common example is available in [decode_cfg.py](https://github.com/LDMX-Software/Hcal/blob/trunk/exampleConfigs/decode_cfg.py)
  `ldmx-sw fire decode_cfg.py hcal_run_XXX_reformat.root`. By default this decoding does not pedestal subtraction. 
