# ESA 2026 HCal detector map

`connections.csv` for `HcalDetectorMap.esa26()`: HCal of the ESA 2026 cosmic
stand, HGCROC 0-3 on one ECON-D. Explicit format: electronics ID
(Polarfire = ECONDDecoder fiber 0, Elink = ECON-D eRx, Channel 0-35) to
HcalDigiID (Section, Layer, Strip, End). HGCROC, RocChannel, Plane and Cable
are for reference.

Sources:
- `LDMX Cosmic Cabling Convention.xlsx` (UVA): HGCROC channel, CMB, plane
  and cable of each bar end; top-down drawings of the 3-quad and 2-quad
  planes; side views with the ROC per station; "Correct polarity" column
  (CMBs installed with the channel order reversed within the cable).
- eRx to HGCROC half measured on run 209 (pedestals per eRx match one pflib
  ROC half each): eRx 0-5 = ROC3 upper, ROC3 lower, ROC0 upper, ROC0 lower,
  ROC2 lower, ROC2 upper. ROC1 (read out from run 462): eRx 6 = lower,
  eRx 7 = upper (pflib HcalBackplane ROC_ERX_MAPPING_DAQ).

Placement (sections and layers as in the cosmic26_v2 geometry, layers from
the top):
- Planes 1-4: top station (section 1), layers 1-4. Planes 1, 2 are the short
  pair (the ESA25 quads, 1 m bars), planes 3, 4 the long pair.
- Planes 5-8: above the ECal (section 0), layers 1-4.
- Planes 9-12: below the ECal (section 2), layers 1-4.
- "Side 1" planes (ROC0, ROC1) have bars along y, "Side 2" planes (ROC2,
  ROC3) along x. With plane numbers increasing downwards this matches the
  alternation of all three stations in the GDML.
- ROC1 reads plane 8 strips 4-7 (ROC0 reads 0-3) and planes 10 and 12. Its
  plane 6 cables (J4-1, J12-1) are not installed, so plane 6 reads strips
  0-3 only.

248 of 288 channels are mapped: 8 belong to the uninstalled ROC1 plane 6
cables, 32 are the unconnected every-ninth channels. Cable J9-1 (ROC1
channels 36-39, plane 10 end 0 of strips 0-3) is mapped but gave no hits in
runs 549-564.

Strip order corrected with cosmic tracks (run 227, zero suppression, 4.8 h):
in the stations above and below the ECal one end of each bar had its strip
order reversed within each group of 4 bars, so the two ends landed on
mirrored bars. Tracks anchored in the top station (where the ends agree)
vote for the end on the straight line: the reversed end is end 0 on planes
5, 9 and 11 (cables J4, J1-J3, J5-J7) and end 1 on plane 7 (J16). After the
correction the two ends agree in all but 3 of about 4000 two-end hits.
These are the cables marked with reversed polarity in the cabling sheet.

Planes 3, 4, 6 and 8 are reversed within each group of 4 bars (both ends),
as found in the TS + HCal alignment (track residuals in x and y) and as the
cabling sheet's polarity column gives. Planes 6 and 8 checked independently
with ROC1 (runs 549-564): referenced to plane 12, the old order
anti-correlates (-1.00) with the bar position and the new one correlates
(+1.00); plane 8 is then one continuous sequence across ROC0 (strips 0-3) and
ROC1 (strips 4-7).

ROC1 order (runs 549-564, HCal-triggered ZS): drawing order with left = low
strip, reversed within the cable where the sheet marks the polarity as
reversed. The two ends of each bar pair up in 84-93% of the events, planes
10 and 12 agree within one strip in 99.4% of the events, and HCal x tracks
from planes 1, 3, 6 and 8 give the direction.

**Provisional:** the drawings fix left/right only relative to the front.
Front is taken at -y (right at +x). If the front is at +y, the strip order
and the two ends flip in every layer.
