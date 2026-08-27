# Real-data tracker reconstruction (ESA slice test / `ldmx-reduced-v3`)

_Living progress document. Last updated: 2026-07-24._

Carries the real-data tracker readout from raw bytes through to geometry-aware
`Measurement`s, reusing the existing MC reconstruction wherever the physics is
identical. Target geometry is the 2-station ESA slice test
(`Detectors/data/ldmx-reduced-v3`): each station has a **stereo** sensor
(upstream) and an **axial** sensor (downstream); a single FEB (0) services all
four hybrids.

---

## Data flow

The two chains diverge at the raw-hit level (different inputs) and converge at
`FittedSiStripHit`, after which the geometry-aware path is shared.

```
   MC (charge-digi)                         DATA
   ────────────────                         ────
 SimTrackerHit                          TrackerRawData (bytes)
   │ DigitizationProcessor                 │ RawTrackerDecoder
   ▼                                        ▼
 SimSiStripHit                          RawSiStripHit
   │                                        │ PedestalSubtractor
   │                                        ▼
   │                                     RawSiStripHit  (pedestal-subtracted)
   │                                        │ SiStripWaveformBuilder
   │                                        ▼
   │                                     SiStripWaveform          ← data-only stage
   │ StripFitProcessor                      │ SiStripWaveformFitProcessor
   │   (fixed noise, addr from hit)         │   (pedestal noise, addr from DAQ map)
   ▼                                        ▼
   └──────────►  FittedSiStripHit  ◄────────┘           ← convergence point
                        │ StripClusterProcessor
                        ▼
                   Measurement (StripMeasurements)          ← scope ends here
```

The extra data-side stage exists because the DAQ splits one channel's readout
across APV trigger frames; `SiStripWaveformBuilder` regroups them into a full
per-channel waveform. MC digitization already emits the assembled samples.

---

## Components

### Event data model (`Tracking/include/Tracking/Event`)

| Class | Role | Notes |
|-------|------|-------|
| `SiStripHit` | abstract base: samples + time | — |
| `SimSiStripHit` | MC digitized hit | + layer/strip + truth |
| `RawSiStripHit` | real-data raw hit | + electronics ids + flags |
| `SiStripWaveform` | assembled per-channel waveform (data) | samples + `feb/hybrid/pchannel`; **no fit result** (moved out) |
| `FittedSiStripHit` | pulse-fit result (both chains) | + `noise` field; optional truth |

### DAQ map (`Tracking/Reco/TrackerDaqMap`, `Tracking/data/daqmap_esa25_slice_test.json`)

The single piece connecting electronics addressing to geometry:
`(feb, hybrid) → (layer_id, n_strips, first_strip, reversed)`. Loaded from JSON
now; kept format-agnostic in memory so it can move onto the conditions DB later.
`SiStripChannelMap` provides the `pchannel ↔ (apv,channel)` inverse and the
`stripId` transform.

Current map (all on FEB 0):

| hybrid | station | orientation | surface (`layer_id`) | beam z |
|:------:|:-------:|:-----------:|:--------------------:|-------:|
| 0 | 1 | stereo | 3100 | −243.5 mm |
| 1 | 1 | axial  | 3101 | −237.5 mm |
| 2 | 2 | axial  | 3201 | −137.5 mm |
| 3 | 2 | stereo | 3200 | −143.5 mm |

### Processors (`Tracking/Reco`, `Tracking/Digitization`)

- **`SiStripWaveformBuilder`** — groups trigger frames into `SiStripWaveform`s
  (two-stage significance threshold). No longer fits.
- **`SiStripWaveformFitProcessor`** — real-data counterpart of
  `StripFitProcessor`: maps address via the DAQ map (rejecting unmapped hybrids
  and out-of-range strips before fitting), fits the shared `StripPulseFitter`
  with per-channel pedestal noise, emits `FittedSiStripHit`.
- **`StripClusterProcessor` / `StripClusterer`** — unchanged clustering, now
  using per-hit noise (`getNoise()`) and, with a DAQ map, the real per-sensor
  strip count for the local-U origin. Both default to the MC constants when
  unset, so the MC path is byte-identical.

---

## Geometry fix (`Detectors/data/ldmx-reduced-v3/recoil.gdml`)

Two bugs relative to the ESA hardware were corrected on 2026-07-23:
- **z-order**: the stereo sensor now sits upstream and the axial downstream in
  each station (previously reversed).
- **stereo sign**: both stations now carry the same stereo tilt (`recoil_l14_rot`
  no longer alternates sign).

Verified by dumping the Acts surfaces: `3100/3200` stereo (+5.72958°) upstream,
`3101/3201` axial downstream. The DAQ map surface ids were updated to match, so
hits sit at physically correct positions with no compensation applied.

---

## Example configs (`Tracking/exampleConfigs`)

| Config | Does |
|--------|------|
| `decode_to_waveforms.py` | bytes → `SiStripWaveform`s (+ optional fit) |
| `waveforms_to_measurements.py` | bytes → `StripMeasurements` (full data chain) |
| `dump_geometry.py` | dump every Acts surface to CSV (geometry ground truth) |
| `plot_waveforms.py`, `plot_measurements.py` | standalone PyROOT diagnostics |

---

## Data samples and thresholds

**Use `Run_182_20251213_174649.dat`** — the golden run. `Run_037_20251210_145218.dat`
(the old config default) is mostly beam-off: in its first 300 events only 26
contain any signal, with the beam turning on around event 278. Diagnosing
anything on Run_037 is misleading.

### Threshold scan (Run_182, 500 events; 2026-07-23)

`ev4sens` = events with all four sensors hit.

| config | meas | ev4sens |
|--------|-----:|--------:|
| baseline `5σ/4, 3σ/5`, cl `4/3/4` | 6 170 | 407/500 |
| **loose-A** `4σ/3, 2.5σ/3`, cl `3/2/3` | 17 317 | **500/500** |
| loose-B `3σ/2, 2σ/2`, cl `2.5/2/2.5` | 19 622 | 500/500 |

**Loose-A is the recommended working point.** The default thresholds were
discarding roughly 2/3 of real hits. The culprit is the *duration* requirement
in `SiStripWaveformBuilder`, not the σ values: with `tp = 45 ns` and 25 ns
sampling, `min_high_samples=4` demands ~100 ns above 5σ and
`min_consecutive_low=5` demands ~125 ns above 3σ, pushing the effective
amplitude threshold to ~8–10σ. Loose-A is the point at which every event
becomes fully seedable; loose-B buys only +13% measurements on top.

Thresholds are exposed as flags on `waveforms_to_measurements.py`
(`--high-threshold`, `--min-high-samples`, `--low-threshold`,
`--min-consecutive-low`, `--seed-threshold`, `--neighbor-threshold`,
`--cluster-threshold`). **Note:** the clustering thresholds are shared with the
MC path, so set them per-config rather than changing the C++ defaults.

Open question: loose-A gives ≈9 hits per sensor per event. Whether that
multiplicity is physical needs validation against beam conditions.

## Verification status

- Fit-duplication refactor: `meas.root` **byte-identical** before/after (proves
  the change was mechanical).
- Geometry fix: hits relocated to the corrected surface positions, same
  434-measurement total over the 300-event sample.
- Clean builds throughout. MC reconstruction path unchanged by construction (not
  re-run against a sim sample yet).

---

## Deferred / not in this PR

- Pedestals and the DAQ map → conditions DB (`SimpleCSVTableProvider`, per-run
  IOVs). Both stay on JSON for now.
- Splitting the sim/real reconstruction code paths.
- **Straight-line track finding/fitting on the recoil slice.** Prototyped
  locally against this chain (a `LinearTrackFinder` `input_type="measurements"`
  path) but deliberately held back from this PR — it needs the track-quality
  question below settled first. Two stations give ndf 0 (χ²≈0), so there is no
  goodness-of-fit to cut on, and the seed heuristic is beam-test-specific.

### Other subsystems: trigger scintillator (assessment 2026-07-23)

The trigger scintillator (3 pads) is a natural downstream constraint for the
tracker fit — the role the ECal played in `LinearSeedFinder` — which would break
the ndf-0 / min-slope-bias limitation above. State of the `TrigScint` module:

- **Decoding already exists** and mirrors ours: `QIEDecoder` / `ZCCMDecoder` turn
  a `uint8_t` byte stream into `TrigScintQIEDigis` via a text channel map
  (`elecID → barID`, `TrigScint/data/channelMap_*.txt`). The byte stream comes
  from the **same `Packing.rawio.SingleSubsystemUnpacker`** the tracker uses, so
  a combined unpack → decode sequence is the natural shape.
- **The ESA DAQ writes tracker and TS into the same raw file** as distinct
  subsystems, **in the same physical frame** — so once TS is mapped to mm, its
  points and the tracker measurements are directly comparable with no
  cross-subsystem alignment transform.
- **TS reco does not connect to the geometry** the way the tracker does: it stays
  in bar/pad-index space. `TrigScintCluster` centroids are bar indices, and
  `setCentroidXYZ` is filled with indices (`cz = -99999`, a placeholder). There
  is no GDML/Acts positioning and no mm-space `Measurement` analogue.

**Missing piece to integrate:** a TS bar → global-mm mapping (analogue of the
tracker DAQ map + geometry hookup), using `trig_scint.gdml` module positions, to
emit TS space points in the shared frame. Then the straight-line fit can take a
TS point as a third constraint. HCal is a later input still.
