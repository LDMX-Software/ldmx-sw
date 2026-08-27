Tracker real-data waveform processing chain
============================================

This directory contains a runnable example of the **real-data** (test-beam /
Rogue) tracker reconstruction chain, from raw `.dat` bytes through assembled
per-channel waveforms, plus a self-contained plotting script.

> This is the real-data counterpart to the simulation chain in
> `Tracking/python/full_tracking_sequence.py`
> (`DigitizationProcessor → StripFitProcessor → StripClusterProcessor → …`).
> The two are designed to converge downstream at `FittedSiStripHit` /
> `Measurement`; the real-data pulse-fit step that bridges the two is not yet
> implemented (see "Where this fits" below).

Pipeline
--------

```
                          ┌─ baseline (pedestal) run ─┐
SingleSubsystemUnpacker ─▶│ RawTrackerDecoder         │─▶ PedestalCalculator ─▶ pedestals.json
   (Packing)              └───────────────────────────┘        (analyzer)

                                                      pedestals.json
                                                            │
                                            TrackerPedestalProvider  (conditions service)
                                                            │  getCondition<TrackerPedestals>
                          ┌──────────── physics run ────────┴──────────────────────────┐
SingleSubsystemUnpacker ─▶│ RawTrackerDecoder ─▶ PedestalSubtractor ─▶ SiStripWaveformBuilder │─▶ *.root
   (Packing)              └────────────────────────────────────────────────────────────┘
                                  RawSiStripHits     TrackerHits          TrackerWaveforms
                                                  (RawSiStripHit)        (SiStripWaveform)

   *.root ─▶ plot_waveforms.py (standalone PyROOT) ─▶ PNGs
```

| Collection         | Type                          | Produced by             |
|--------------------|-------------------------------|-------------------------|
| `TrackerRawData`   | `std::vector<uint8_t>`        | `SingleSubsystemUnpacker` |
| `RawSiStripHits`   | `std::vector<RawSiStripHit>`  | `RawTrackerDecoder`     |
| `TrackerHits`      | `std::vector<RawSiStripHit>`  | `PedestalSubtractor` (pedestal-subtracted samples) |
| `TrackerWaveforms` | `std::vector<SiStripWaveform>`  | `SiStripWaveformBuilder` |

Pedestals and per-channel noise are **not** stored in the event — they are a
conditions object (`TrackerPedestals`) served by `TrackerPedestalProvider`, the
same way Ecal/Hcal deliver their HGCROC pedestals/gains. `PedestalSubtractor`
queries it for the per-sample means; `SiStripWaveformBuilder` queries it for the
per-channel noise used in the significance cuts. The raw and subtracted hit
collections are both `RawSiStripHit`, distinguished only by name.

Prerequisites
-------------

1. A built + installed ldmx-sw (`just build`).
2. The raw `.dat` files must be **mounted into the denv container** (anything
   outside the workspace is not visible by default):

   ```bash
   just mount /sdf/data/hps/users/mgignac/hardware/data/LDMX
   ```

Step 1 — compute pedestals (once per baseline run)
--------------------------------------------------

Pedestals **must** come from a dedicated baseline/pedestal run, *not* a physics
run. `PedestalCalculator` accumulates per-channel, per-sample mean and RMS noise
and writes `pedestals.json`.

```bash
just fire Tracking/exampleConfigs/compute_pedestals.py -- \
    --dat /sdf/data/hps/users/mgignac/hardware/data/LDMX/Run_049_20251211_093048.dat \
    --max-events 500
# → [PedestalCalculator] Wrote 2560 channels from 500 events to 'pedestals.json'
```

2560 channels = 4 hybrids × 5 APV25 × 128 channels — a full tracker readout.

Step 2 — decode → subtract → build waveforms (per physics run)
--------------------------------------------------------------

```bash
just fire Tracking/exampleConfigs/raw_to_measurements.py -- \
    --dat /sdf/data/hps/users/mgignac/hardware/data/LDMX/Run_182_20251213_174649.dat \
    --pedestal-file pedestals.json \
    --max-events 5 \
    --stop-at fit \
    --output tracker_waveforms_run182.root
# → [TrackerPedestalProvider] Loaded 2560 channel pedestals from 'pedestals.json'
# → [SiStripWaveformBuilder] Built N waveforms (>=4 samples @5s, streak>=5 @3s) from 25800 hits
```

`SiStripWaveformBuilder` groups all APV-trigger hits for each
`(feb, hybrid, pchannel)`, sorts by trigger, concatenates into a
`n_triggers × 3`-sample waveform, and keeps a channel only if it passes a
two-stage significance cut.

Useful flags:

| Flag | Default | Meaning |
|------|---------|---------|
| `--high-threshold` | 5.0 | per-sample ADC/noise for the high-threshold count cut |
| `--min-high-samples` | 4 | min samples that must exceed `high_threshold` |
| `--low-threshold` | 3.0 | per-sample ADC/noise for the consecutive-streak cut |
| `--min-consecutive-low` | 5 | min consecutive samples exceeding `low_threshold` |
| `--n-triggers` | 10 | expected APV triggers per RoR |
| `--verbose-waveforms` | off | print ASCII traces of every kept waveform |

Step 3 — plot (standalone, reads the ROOT file directly)
--------------------------------------------------------

`plot_waveforms.py` is **not** an ldmx processor — it is a plain PyROOT +
matplotlib script that loads the Tracking Event dictionary, reads the
`TrackerWaveforms` collection straight out of the `LDMX_Events` tree, and writes
PNGs (multi-panel overview, per-waveform panels, and strip occupancy).

```bash
denv python3 Tracking/exampleConfigs/plot_waveforms.py tracker_waveforms_run182.root \
    --max-waveforms 6
# → <stem>_all.png, <stem>_wf##_....png, <stem>_occupancy.png
```

Interpreting the output / threshold tuning
------------------------------------------

The cuts are expressed in **units of per-channel noise σ** (sample value ÷ the
RMS noise from `pedestals.json`), so they are gain-independent.

Sanity checks observed on real data:

- **Signal run (Run 182):** at the default `5σ`/`3σ` cuts, peak significances
  reach **~25–47σ** above baseline (median ~17σ) — real, large APV25 pulses.
- **Quiet / no-beam events:** produce **0 waveforms** at default cuts — nothing
  clears `5σ`. This is expected, not a failure; loosen the thresholds
  (e.g. `--high-threshold 2 --min-high-samples 2 --low-threshold 1.5
  --min-consecutive-low 2`) to inspect near-noise channels.

Caveats:

- The per-channel noise is the population RMS of the raw ADC samples in the
  baseline run. If the pedestal run is not a clean, quiet baseline (e.g. common-
  mode swings), the noise estimate is inflated and the `Nσ` cuts become too
  tight. Common-mode subtraction before the noise estimate is a likely future
  improvement.
- `pedestals.json` is keyed by `feb:hybrid:apv:channel`; a physics run decoded
  with a mismatched pedestal file will log `no pedestal for channel …` warnings
  and pass those hits through with `noise = 0` (which the builder then skips).

Where this fits in the larger tracking workflow
-----------------------------------------------

This chain currently ends at `SiStripWaveform`. To join the shared Acts-based
track-finding spine (`SeedFinderProcessor → CKFProcessor → …`), the real-data
waveforms need to become `FittedSiStripHit`s (amplitude + t0 per strip, the same
type the simulation produces via `StripFitProcessor`), after which the existing
`StripClusterProcessor` — which already "runs on both real data and simulation"
— turns them into `Measurement`s. The missing pieces are a waveform pulse-fit
step and a `(feb,hybrid,pchannel) → (layer,strip)` geometry mapping.
