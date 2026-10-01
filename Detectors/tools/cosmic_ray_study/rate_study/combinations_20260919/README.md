# Aligned ESA cosmic-muon rate study — 19 September 2026

Authors: Emrys Peets, Matthew Gignac, Takumi Britt.

Presentation revised 20 September 2026; numerical results are unchanged from
19 September. The new geometry overview uses `make_overview.py` and the original
vector ray image, with their different ECAL placements identified explicitly.
`figure_sequence.tex` places that overview immediately before the combined
active-layer and retained-rate figure. Bold cardinal-red rate pairs beside the
selection labels are ordered no roof / 3 ft concrete, in muons per hour.

The updated report is `../cosmic_rate_report.pdf`. All devices remain installed.
ECAL is translated by (+46.55962243, -46.0762891361, 0) mm to align its active
envelope with the tracker/TS/LYSO axis. No vertical assembly shift is applied.
The prior PDF and its source are preserved in `previous_report/`.

## Main rates

| Selection | No roof, per hour | 3 ft concrete, per hour |
|---|---:|---:|
| Golden: every active layer | 2.163187 | 1.842396 |
| Every layer except ECAL required | 2.195328 | 1.869764 |
| Golden with any one tracker layer unrestricted | 2.163187 | 1.842396 |
| All ten HCAL layers, central hits unrestricted | 13259.61 | 11290.19 |

These are muon-crossing estimates from a sea-level flux and straight-ray CSDA
model. They do not include scattering, detector response, noise, accidentals or
live time. Numeric standard errors in the CSV files are integration errors, not
the physical uncertainty. Detector mass is integrated ray by ray but represented
by a polystyrene CSDA stopping proxy; concrete has its own range table.

## Rebuild

From the parent report directory:

```sh
PYTHON=/path/to/python3 bash build_report.sh
```

Dependencies: Python >=3.9, NumPy >=1.26, SciPy >=1.13, Matplotlib >=3.8, a C++17
compiler (`CXX` can select it), and Tectonic. No network fetches are needed for the
rate calculation; range tables and the geometry snapshot are bundled. Tectonic
may fetch TeX packages on its first use. ROOT is optional for reproducing the
independent solid-validation checks:

```sh
RUN_ROOT_VALIDATION=1 PYTHON=/path/to/python3 bash build_report.sh
```

Default: eight Sobol scrambles, 2^17 rays each, for each of 39 endpoint/directional
families. The main equipment integration samples 40,894,464 rays. Trigger/singles
calculations add 17 × 8 × 2^17 rays. Two strict station unions are derived from
the equipment samples; two loose station unions use the trigger samples.
Random seeds are fixed and recorded. Checkpoints are keyed by input and code
hashes, sample size and sampler type. Deleting `results/checkpoints/` forces a
fresh integration. Report-only regeneration uses:

```sh
python3 combinations_20260919/make_assets.py
tectonic -X compile cosmic_rate_report.tex --keep-logs
python3 combinations_20260919/package_manifest.py
```

## Tables and bit conventions

All rates and numerical SE columns are per hour. `geometric` is the separate
70 cos²(theta) reference above 1 GeV/c without attenuation. `no_roof` and
`concrete` use the energy spectrum and material thresholds. `half_column` and
`double_column` rescale only detector mass in the stopping model.

- `results/specified_subsets.csv`: 896 rows. Bits in `id` are, low to high:
  H_T, H_M, H_B, S, Y, E, T1, T2, T3, T4. At least one HCAL station is required.
  A selected station requires every active layer in that station. Selected
  tracker bits require those named layers; all unselected hits are unrestricted.
- `results/inclusive_combinations.csv`: 280 rows. `hcal_mask` uses bits 1,2,4 for
  top, above-ECAL, below-ECAL stations. S/Y/E equal 1 means required, 0 means
  unrestricted. `tracker_k` means at least k of four tracker layers. These are
  overlapping inclusive samples and must not be summed.
- `results/exclusive_tracker_counts.csv`: same 280 settings, but `tracker_k`
  means exactly k tracker hit conditions. Exact counts partition the corresponding
  zero-tracker-requirement sample. Small differences retain their signed estimate
  and numerical SE; do not interpret an unresolved zero as an efficiency limit.
- `results/hcal_any_layer_triggers.csv`: seven loose station coincidences,
  ten individual layer singles, loose any-station/at-least-two-station unions,
  and strict any-station/at-least-two-station unions (21 rows).
- `results/*replicates.npz`: per-scramble estimates retain correlations of rows
  sharing samples. Rates in different tables are generally not independent.

Tracker layers T1--T4 are ordered from highest to lowest: CAD Z = 3309.888,
3303.888, 3209.888, 3203.888 mm. The 26 layer bits use descending Z order, as
recorded in `legacy_geometry.prepare` and the report inventory.

For station ORs, the earliest crossed layer completes a station. The last
required station completion sets the material stopping plane. All material
above that plane remains; no downstream traversal is imposed. The 10 MeV
residual kinetic energy is a range-table convention, not an electronics cut.

## Code and validation

`calculate_combinations.py` chooses complete sampling domains separately for
each required selection. It never extrapolates a TS/LYSO-conditioned sample to
the broad HCAL acceptance. `calculate_hcal_triggers.py` implements station ORs
and layer singles. `ray_geometry.py/.cpp` compute exact chords in the frozen
convex-prism scene, with four same-material tracker daughters counted only once
for stopping mass. The local `legacy_geometry.py` freezes the previously
validated layer preparation and Python intersection implementation.

`validate_ray_geometry.py`, `validate_numerics.py`, and
`validate_station_angles.py` reproduce the independent checks. Their outputs
are in `review/`. The delivered geometry agrees with ROOT for all 384 checked
rays; 16,384 broad rays agree with the independent Python hit implementation.
All subset and multiplicity monotonicity checks pass. Independent pseudorandom
and broader-angle sampling checks pass. The TS/LYSO envelope is analytically
inside every active tracker polygon, explaining the tracker-relaxation plateau.

The geometry provenance and historical scans remain in the parent directory.
`../SHA256SUMS` identifies included artifacts, and `../delivery_manifest.json`
records author names, runtime versions and the calculation fingerprint. The
PDF visual/text QA record names the exact delivered PDF hash. No native Geant4
transport or detector-efficiency calibration was performed for this update.
