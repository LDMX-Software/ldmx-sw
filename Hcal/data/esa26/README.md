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
- ROC1 reads planes 6 and 8 strips 4-7 (ROC0 reads 0-3) and planes 10 and
  12. The cabling sheet lists its plane 6 cables (J4-1, J12-1) as not
  installed, but they read real bars (see below).

256 of 288 channels are mapped; the other 32 are the unconnected every-ninth
channels. Cable J9-1 (ROC1 channels 36-39, plane 10 end 0 of strips 0-3) is
mapped but gave no hits in runs 549-600.

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

Planes 1 and 2 (the short pair) are reversed within each group of 4 bars as
well, to match planes 3 and 4 below them: in run 227 (the only run with their
inputs read out, see below) plane 1 strip s paired with plane 3 strip
4*(s/4) + 3 - s%4, and plane 2 with plane 4 the same way; with the reversal
both pairings are diagonal. Their bar ends pair up in 73-85% of the events
and their pulse heights match the other planes, so they are real bars.

Check of every plane with HCal-only straight tracks (leave-one-out residual
of the plane, from the other planes of its view, as mapped vs reversed
within each group of 4 bars; fraction within 50 mm): runs 556-564, planes
5-12 0.97-0.99 vs 0.22-0.28, plane 3 0.65 vs 0.26, plane 4 0.69 vs 0.26;
run 227, plane 2 0.99 vs 0.25, plane 4 0.91 vs 0.23. Plane 1 has too few x
tracks in run 227 and follows plane 3 through the strip pairing above.
Online, their inputs (ROC0 and ROC2 channels 0-12, 36-48) have been switched
off in the trigger and masked in zero suppression since 4-Nov, because they
read far above pedestal with the SiPM bias on; runs after that have no
planes 1 and 2 data.

ROC1 order (runs 549-564, HCal-triggered ZS): drawing order with left = low
strip, reversed within the cable where the sheet marks the polarity as
reversed. The two ends of each bar pair up in 84-93% of the events, planes
10 and 12 agree within one strip in 99.4% of the events, and HCal x tracks
from planes 1, 3, 6 and 8 give the direction.

ROC1 plane 6 (runs 594 and 600): with one hit on each plane, channel 13+k of
J4-1 and 49+k of J12-1 pair with plane 8 strip 4+k in 67-86% of the events
and with a neighbor in nearly all the rest (under 1% two or more strips
off), so they are
strips 4-7 in channel order, J12-1 end 0 and J4-1 end 1 as on ROC0. Strip 4
is also the bar that fires together with ROC0's strip 3. The two ends pair
up in 99% of the events.

**Provisional:** the drawings fix left/right only relative to the front.
Front is taken at -y (right at +x). If the front is at +y, the strip order
and the two ends flip in every layer.
