# Trigger
Software emulation and firmware implementation of LDMX data triggers.

### Table of Contents
- `Algo`: an ldmx-sw module implenting software emulation of triggers
- `Algo_HLS`: firmware implementation of triggers
  - _Note_: kept alongside Algo because parts of the implementation can
    be compiled and used directly withing `Algo`.
- `HLS_arbitrary_Precision_Types`: code to emulate firmware-level arbitrary precision types
- `ruckus`: build system for firmware
