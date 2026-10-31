# ESA 2026 ECal detector map

Tables for `EcalDetectorMap.esa26()`, ECal layers 2 and 3 of the ESA 2026
cosmic stand (one HD module per layer, module 0).

- `layer_map.csv`: L2 is optical link 2 (layer 2), L3 is link 3 (layer 3),
  the `fiber` of the ECONDDecoder.
- `motherboard_map.csv`: ECON-D eRx (POLARFIRE_ELINK) to HGCROC and half.
  Measured on run 209: the mean SOI ADC of each eRx matches exactly one ROC
  half of the pflib decoding. L2 (type 2) reads 12 eRx, L3 (type 3) reads
  ROC0, 2, 3 in 6 eRx.
- `cell_map.csv`: HGCROC channel to module cell. Channel to pad from the CMS
  HD hexaboard map (`hd_pad_to_channel_mapping_V2p1`), pad to cell by
  position: pads rotated by +30 degrees, scaled to the cell pitch, nearest
  cell. All 432 pads land in distinct cells (largest offset 1.2 mm).

**Provisional:** six orientations (3-fold rotation, with or without a
mirror) fit equally well; the one used here is an assumption until it is
checked with tracks or a reference map.
