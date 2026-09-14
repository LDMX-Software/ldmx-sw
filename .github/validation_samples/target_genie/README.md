# Generating GENIE electron-mode cross-section splines

The `target_genie` validation sample requires pre-computed GENIE
cross-section splines for electron scattering on all isotopes present in
the detector geometry.

Splines are stored as **one file per GENIE tune**, with the tune name in
the filename:

```
$CI_DATA/target_genie/gxspl_emode_<TUNE>.xml
```

`config.py` derives the spline filename from `my_sim.genie_nuclear.tune`,
so the two can never disagree (see "Tune ↔ file matching" below).

## Isotopes included

| Element  | Isotopes (PDG codes)                                         |
|----------|--------------------------------------------------------------|
| free-n   | 1000000010                                                   |
| H        | 1000010010 (H-1), 1000010020 (H-2)                          |
| C        | 1000060120 (C-12), 1000060130 (C-13)                        |
| O        | 1000080160 (O-16), 1000080170 (O-17), 1000080180 (O-18)     |
| Si       | 1000140280 (Si-28), 1000140290 (Si-29), 1000140300 (Si-30)  |
| Ti       | 1000220480 (Ti-48, dominant isotope only)                    |
| W        | 1000741820 (W-182), 1000741830 (W-183), 1000741840 (W-184), 1000741860 (W-186) |

The GDML geometry defines elements with just Z and average A (e.g.
`<element Z="74" ...>`), so Geant4 auto-expands to all natural isotopes
via NIST tables.  Only the dominant isotopes are included here; minor
ones (W-180 at 0.12%, Ti-46/47/49/50) are skipped at runtime with a
warning but the physics impact is negligible.

H, C, O, Si isotopes are included to suppress the "No cross-section
spline available" warnings from detector materials in the target region.

## Available tunes

These are the five tunes the original master spline file carried.  Each
is generated into its own file:

| Tune              | File                                  |
|-------------------|---------------------------------------|
| `G18_02a_00_000`  | `gxspl_emode_G18_02a_00_000.xml`      |
| `G18_02a_02_11b`  | `gxspl_emode_G18_02a_02_11b.xml` (default in `config.py`) |
| `G18_02b_02_11b`  | `gxspl_emode_G18_02b_02_11b.xml`      |
| `G21_11a_00_000`  | `gxspl_emode_G21_11a_00_000.xml`      |
| `G21_11b_00_000`  | `gxspl_emode_G21_11b_00_000.xml`      |

You only need the file for the tune you actually run.  The validation
sample uses `G18_02a_02_11b`.

## Tune ↔ file matching (important)

GENIE's `XSecSplineList` indexes splines **by tune name**.  If the spline
file was generated under a different tune than the one requested at
runtime, GENIE finds *no* matching splines and silently recomputes every
cross section on the fly — in particular the RES resonance-excitation
cache — which makes the job appear to **hang for many minutes** with no
error.

To make this impossible, `config.py` builds the filename from the tune:

```python
my_sim.genie_nuclear.tune = "G18_02a_02_11b"
my_sim.genie_nuclear.spline_file = (
    f"{os.environ['CI_DATA']}/target_genie/gxspl_emode_{my_sim.genie_nuclear.tune}.xml"
)
```

To switch tunes, change only `my_sim.genie_nuclear.tune`; the matching
file is selected automatically (it must of course exist — generate it
first).

## How to regenerate

Run inside the ldmx-sw `denv` container, from the repo root.  This loops
over all five tunes and writes one file each into `$CI_DATA/target_genie/`:

```bash
cd /path/to/ldmx-sw
export CI_DATA=/path/to/ci-data   # if not already set

TARGETS=1000000010,1000010010,1000010020,1000060120,1000060130,1000080160,1000080170,1000080180,1000140280,1000140290,1000140300,1000220480,1000741820,1000741830,1000741840,1000741860
MSG="$PWD/.github/validation_samples/target_genie/Messenger_ErrorOnly.xml"

for TUNE in G18_02a_00_000 G18_02a_02_11b G18_02b_02_11b G21_11a_00_000 G21_11b_00_000; do
  echo ">>> generating $TUNE"
  denv gmkspl \
    -p 11 \
    -t "$TARGETS" \
    -n 30 \
    -e 10 \
    --tune "$TUNE" \
    --event-generator-list EM \
    -o "$CI_DATA/target_genie/gxspl_emode_${TUNE}.xml" \
    --message-thresholds "$MSG"
done
```

To regenerate only the default tune, run the loop body with
`TUNE=G18_02a_02_11b`.

This is a long run (the `G18_02b`/`G21_11` families pull in extra models
and are slower); consider `tmux`/`nohup` or running overnight.  Each tune
writes its own file, so a failure in one does not lose the others.

### Parameters

- `-p 11` — electron probe (PDG code 11)
- `-t ...` — comma-separated list of target isotope PDG codes
- `-n 30` — 30 knots per spline
- `-e 10` — maximum energy 10 GeV (sufficient for 8 GeV beam)
- `--tune <TUNE>` — GENIE tune; baked into the output filename so it
  always matches the runtime tune (see "Tune ↔ file matching" above)
- `--event-generator-list EM` — electromagnetic interaction processes only (QEL, RES, DIS, MEC)
- `--message-thresholds Messenger_ErrorOnly.xml` — suppress verbose GENIE output

### Adding new isotopes

If the detector geometry changes and new elements appear in the
`target_region` volume, runtime warnings of the form:

```
No cross-section spline available for target XXXXXXXXXX (Z=..., A=...)
```

indicate that the corresponding PDG code needs to be added to the
`TARGETS` list above and the splines regenerated (for every tune you use).
