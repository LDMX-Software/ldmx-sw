# ESA cosmic-ray geometry and rate study

Emrys Peets, Matthew Gignac and Takumi Britt. Rate calculation: 19 September
2026; report and figures revised 20 September 2026.

Read the [study PDF](rate_study/cosmic_rate_report.pdf) for the aligned-ECAL
geometry, HCAL trigger comparisons, detector combinations, assumptions and
validation. This directory contains the complete reproducible study, including
the earlier placement comparisons. The adjacent
[cosmic viewer](../cosmic_viewer/README.md) provides the interactive stand display
and the modular geometry already on this branch.

## Get the study

```sh
git clone --branch epeets https://github.com/LDMX-Software/ldmx-sw.git
cd ldmx-sw/Detectors/tools/cosmic_ray_study
python3 verify_release.py
```

For an existing checkout, switch to `epeets` and run `git pull --ff-only` before
entering this directory. No full ldmx-sw build, ROOT installation or CAD source
file is needed to reproduce the rate integration. The bundled GDML, geometry
snapshots and stopping-range tables are sufficient.

## What this adds to the HCal test-stand work

This continues [issue #2119, Add HCal teststand geometry](https://github.com/LDMX-Software/ldmx-sw/issues/2119)
and the geometry/viewer work in [PR #2159](https://github.com/LDMX-Software/ldmx-sw/pull/2159).
The existing CAD-registered stand contains three HCal stations: two layers of
twelve bars at the top, four layers of eight above ECal, and four layers of
twelve below it, for **104 bars in ten layers**. Bar dimensions, aluminum covers
and packed copy numbers come from the HCal development branch; the 12/8/12
counts remain CAD-based assumptions pending hardware confirmation.

The study turns that geometry into explicit commissioning-rate comparisons:

- Align the ECAL active envelope with the tracker axis at its original height.
- Compare strict HCAL layer coincidences, any-layer station triggers, station
  unions and individual-layer singles.
- Tabulate 896 specified equipment subsets, 280 inclusive multiplicity
  combinations and 280 exact tracker-count combinations, including the golden
  event requiring every active layer.
- Integrate installed material separately along each ray. Broad HCAL-only
  selections have their own sampling domains; they are not extrapolated from
  the narrow TS/LYSO acceptance or assigned the golden event's material column.
- Preserve the source, seeds, per-scramble results, input hashes and independent
  geometry/numerical checks so others can reproduce or revise the estimates.

This extends the geometry and rate-planning part of #2119. It does **not** close
the remaining HCal readout-position, sensitive-detector, digitization or
reconstruction work. The existing geometry smoke configuration disables
sensitive detectors. No new native Geant4 transport validation is claimed for
the aligned rate model; whole-detector overlaps and surveyed registration still
require review.

## Geometry and event definitions

The numerical input is
[`aligned_scene.json`](rate_study/combinations_20260919/inputs/aligned_scene.json).
It includes 356 placed objects, 217 active volumes and 26 active layers. The
ECAL assembly is translated by **(+46.55962243, -46.0762891361, 0) mm** relative
to the original offset layout. Other equipment stays in place. CAD supports
appear in the viewer but are outside the rate model's material inventory.

To write a self-contained set of GDML files for the aligned layout:

```sh
python3 export_aligned_geometry.py --output build/aligned_geometry
```

Load `build/aligned_geometry/detector.gdml` with its neighboring GDML files
present. The exporter records the source hashes and checks the translation
against every object in the frozen scene. The historical offset geometry stays
in `rate_study/geometry/`; the shared viewer's default layout is also the
historical offset layout. The report's historical accepted-ray panel is labeled
accordingly. Do not substitute that offset scene when reproducing aligned rates.

“ECAL unrestricted” and “no tracker requirement” relax hit requirements while
leaving those devices and their material installed. Hits and misses in an
unrestricted device are both accepted; these selections are not vetoes. A
**golden cosmic event** crosses all 26 active layers. “HCAL only” below requires
all ten HCAL layers and imposes no central-detector hit requirements.

| Selection | No roof, per hour | 3 ft concrete, per hour |
|---|---:|---:|
| Golden cosmic event | 2.163187 | 1.842396 |
| Every layer except ECAL required | 2.195328 | 1.869764 |
| Golden with any one tracker layer unrestricted | 2.163187 | 1.842396 |
| HCAL only | 13259.61 | 11290.19 |

These are conditional muon-crossing estimates with straight rays and a
continuous-slowing-down stopping model. They omit scattering, detector
efficiency, electronics thresholds, accidentals and dead time. Mixed detector
material uses a polystyrene stopping proxy; concrete has its own range table.
The tabulated numerical errors measure integration precision, not the physical
uncertainty. Inclusive selections overlap and must not be summed. See the
[table definitions](rate_study/combinations_20260919/README.md#tables-and-bit-conventions)
for masks, exact tracker counts and correlations.

## Reproducing the study — Section 6 of the report

The instructions below reproduce Section 6, with paths made explicit for this
repository. The report directory contains the original geometry provenance,
saved earlier studies and the calculation in `combinations_20260919/`.

From `Detectors/tools/cosmic_ray_study`, create a Python environment. Python 3.10
or newer is a convenient choice for installing the listed package versions.
A C++17 compiler must be available as `c++` (or set `CXX`), and Tectonic must be
on `PATH` to build the PDF.

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r rate_study/requirements.txt
cd rate_study
PYTHON=python bash build_report.sh
```

Python requires NumPy, SciPy and Matplotlib. The small prism-intersection
library compiles on first use. ROOT is used only by the additional geometry
validation script, not by the rate integrator. Tectonic may download its TeX
packages on first use; the rate calculation itself uses bundled inputs.

The default calculation uses eight scrambles of `2^17` rays per anchor family.
Checkpoints allow interrupted integrations to resume. The main equipment study
uses 39 families (40,894,464 rays); the additional trigger/singles calculations
use 17 families. This is the full calculation, not a quick smoke test. Source
files, input snapshots, numerical outputs and the PDF are identified by the
SHA-256 manifests.

| File in `rate_study/combinations_20260919/` | Purpose |
|---|---|
| `calculate_combinations.py` | 896 specified subsets, 280 inclusive combinations and 280 exact tracker-count combinations |
| `calculate_hcal_triggers.py` | Any-layer station coincidences, station unions and ten individual HCAL-layer singles |
| `ray_geometry.py`, `ray_geometry.cpp` | Active hits and installed-material chords, with tracker material counted once |
| `results/*.csv`, `results/*.json`, `results/*replicates.npz` | Rates, numerical errors, settings and correlated per-scramble values |
| `README.md` | Selection labels, bit definitions and detailed validation instructions |

The previous PDF, source and build entry point are preserved in
`rate_study/combinations_20260919/previous_report/`. Rebuilding the current study
does not require rerunning historical geometry variations or downloading range
tables.

To rebuild just the figures, tables and PDF from the saved numerical results,
run these commands from `rate_study`:

```sh
python combinations_20260919/make_assets.py
tectonic -X compile cosmic_rate_report.tex --keep-logs
python combinations_20260919/package_manifest.py
```

To run the additional ROOT checks as part of the full build:

```sh
RUN_ROOT_VALIDATION=1 PYTHON=python bash build_report.sh
```

The numerical checks can also be run directly:

```sh
python combinations_20260919/validate_numerics.py
```

`verify_release.py` checks the saved files before a rebuild. Regenerating the PDF
can change its binary hash through build metadata; `package_manifest.py`
records the rebuilt file. Keep the frozen source unchanged when comparing
calculation fingerprints: the checkpoint keys include input and code hashes.

## Validation record

The delivered study includes the original independent ROOT/Python intersection
checks, sampling checks, rate identities, PDF inspection and isolated rebuild
records under `rate_study/combinations_20260919/review/`. The publication checks
for this repository are in [`publication_validation.json`](publication_validation.json).
The study PDF is identical to the reviewed desktop report; packaging does not
change any rate results. Formatting is disabled within the frozen study archive
to preserve its source fingerprints and published checksum manifest.
