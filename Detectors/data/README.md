# LDMX Detectors
This directory holds the various LDMX detector descriptions we've used.
We follow a naming convention to simply stay organized and help with
collaboration.

## Naming Convention
`ldmx-<category>-v<version>[-<beam>]`

- All detectors have a `ldmx-` prefix
- The next string defines the "category" of the detector, generally
  thought of as structurally different detectors.
  - The `det` category is the full LDMX detector
  - The category can have hyphens as well.
- The string starting with `-v` is the version number for that detector
- Optionally, a suffix can define the beam energy the detector was
  designed for. This is optional since some detectors (e.g. the one
  used for the test beam) do not change their design when the beam
  energy changes.
  - If no beam energy suffix is provided, the beam energy is assumed
    to either be 4GeV or that the detector would not change structure
    with a different beam.

### Categories
This defines what the different category names mean so that future
collaborators understand what the detector is meant to describe.
- `det`: the full LDMX detector
- `ti`: the full detector but using a Ti target instead of W
- `lyso`: the full detector but using a LYSO target instead of W
- `hcal-prototype`: the HCal prototype detector used during testbeam
- `reduced`: reduced LDMX to be ran in 2024

### Examples
- `ldmx-det-v14` : version 14 of the full LDMX detector for the 4GeV beam
- `ldmx-hcal-prototype-v2.0` : version 2.0 the HCal prototype detector used during testbeam
- `ldmx-ti-v9` : version 9 of the full LDMX detector for the 4GeV beam but with a Ti target (instead of W)
- `ldmx-det-v14-8gev` : version 14 of the full LDMX detector updated for an 8GeV beam

### Archived geometries
Previously used geometries can be found under the `archived` directory. In case you need to use any of these, ran `tar -xf` on them and move them back under `data` and then re-configure and re-install ldmx-sw (all detector directories under `data/` are included in the install location):
```
cd ldmx-sw/Detectors/data
tar -xzf archived/<detector>.tar.gz
just configure build # even if you've already done this before!
```
