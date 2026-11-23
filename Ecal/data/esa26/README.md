# ESA 2026 ECal detector map

Tables for `EcalDetectorMap.esa26()`, ECal layers 0, 2 and 3 of the ESA 2026
cosmic stand (one HD module per layer, module 0).

- `layer_map.csv`: L0 is optical link 0 (layer 0), L2 is link 2 (layer 2),
  L3 is link 3 (layer 3), the `fiber` of the ECONDDecoder.
- `motherboard_map.csv`: ECON-D eRx (POLARFIRE_ELINK) to HGCROC and half.
  Measured on run 209: the mean SOI ADC of each eRx matches exactly one ROC
  half of the pflib decoding. L2 (type 2) reads 12 eRx, L3 (type 3) reads
  ROC0, 2, 3 in 6 eRx. L3 uses logical ROC IDs 10 + ROC, so its channels can
  map to other cells than the same L2 channels (the IDs only join the two
  tables). L0 (type 0) reads ROC1, 2, 4, 5 in 8 eRx, in the same eRx to ROC
  half order as L2 (pflib DAQ eRx mapping), with logical ROC IDs 20 + ROC.
- `cell_map.csv`: HGCROC channel to module cell, from cosmic muons (runs 255,
  271-273, HCal trigger):
  - channel to pad: CMS HD hexaboard map `hd_pad_to_channel_mapping_V2p1`,
    with pflib ROC 0, 1, 2, 3, 4, 5 on hexaboard ASIC 0, 2, 4, 1, 3, 5
    (same in both layers);
  - L3 is mirrored relative to L2 (pad y to -y);
  - module orientation from HCal tracks: pads rotated by 330 degrees after a
    mirror (x to -x) in the global frame, then `flip_x` to the module cell
    frame, nearest cell (largest offset 1.3 mm). L3 cells are taken
    relative to the L3 layer center of `ldmx-esa-cosmic26-v2`, which sits
    +8.79 mm (x) and +0.29 mm (y) from L2: the stand has no designed odd-layer
    shift, the geometry's layer shift carries the L3 position from muons
    (13.6 mm board displacement minus the half cell the flip moves the grid);
  - L0 (from run 484, 4-layer runs): same ROC relabel and orientation as L2
    (muon tracks through L1, L2 and L3: no rotation or mirror relative to L2,
    offset (-1.2, +0.6) mm in the pad frame, 50.5 mm above L2 vs 50.8 mm in
    `ldmx-esa-cosmic26-v2`). Every L0 channel lands on the cell of the same
    L2 channel.

  With this map 71% of the L2-L3 muon pairs are in the same or a neighboring
  cell (provisional map: 3%). Generator and validation: `work/ecal-map26`
  (`make_map26.py`, README there).

The EcalReconConditions (pedestals, gains) live in
[conditions-data](https://github.com/LDMX-Software/conditions-data) under
`Ecal/esa26`, loaded by `LDMX.Ecal.esa26_conditions.esa26_recon_conditions()`.

Open: the module center from the tracks differs from the v2 geometry by about
-99 mm in x and +65 mm in y; it depends on the provisional HCal conventions
(front at -y, planes 6 and 8).
