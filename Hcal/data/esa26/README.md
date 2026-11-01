# ESA 2026 HCal detector map

`connections.csv` for `HcalDetectorMap.esa26()`: HCal of the ESA 2026 cosmic
stand, HGCROC 0, 2, 3 on one ECON-D. Explicit format: electronics ID
(Polarfire = ECONDDecoder fiber 0, Elink = ECON-D eRx, Channel 0-35) to
HcalDigiID (Section, Layer, Strip, End). HGCROC, RocChannel, Plane and Cable
are for reference.

Sources:
- `LDMX Cosmic Cabling Convention.xlsx` (UVA, 31-Oct-26): HGCROC channel,
  CMB, plane and cable of each bar end; top-down drawings of the 3-quad and
  2-quad planes; side views with the ROC per station.
- eRx to HGCROC half measured on run 209 (pedestals per eRx match one pflib
  ROC half each): eRx 0-5 = ROC3 upper, ROC3 lower, ROC0 upper, ROC0 lower,
  ROC2 lower, ROC2 upper.

Placement (sections and layers as in the cosmic26_v1 geometry, layers from
the top):
- Planes 1-4: top station (section 1). Planes 1, 2 are the short top pair,
  not in the GDML, so not mapped; planes 3, 4 are layers 1, 2.
- Planes 5-8: above the ECal (section 0), layers 1-4.
- Planes 9-12: below the ECal (section 2), layers 1-4. ROC1 (planes 10, 12)
  is not installed.
- "Side 1" planes (ROC0, ROC1) have bars along y, "Side 2" planes (ROC2,
  ROC3) along x. With plane numbers increasing downwards this matches the
  alternation of all three stations in the GDML.

144 of 216 channels are mapped: 64 belong to uninstalled cables, 48 to the
top pair that is not in the geometry, 24 are the unconnected every-ninth
channels.

**Provisional:** the drawings fix left/right only relative to the front.
Front is taken at -y (right at +x). If the front is at +y, the strip order
and the two ends flip in every layer.
