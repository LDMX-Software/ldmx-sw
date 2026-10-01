# Geometry and rate-model review, 19 September 2026

This review checked the frozen rate-study geometry and its relationship to the modular LDMX framework. It does not certify a detector response or a transport run.

## Aligned geometry

Use `../../ecal_comparison/centered_only/scene.json` as the numerical geometry. The complete ECal subtree is translated by (+46.5596224300, -46.0762891361, 0) mm relative to the original rate geometry. Its full silicon-envelope center then lies on the tracker/TS/LYSO transverse axis, (5027.5199931173, -155.7502301020) mm. Heights, rotations, internal sensor offsets, and the other subsystems remain at their original positions. This is the ECal-only centering case; the earlier 200 mm raised-assembly variant is a separate study.

The finite active-volume requirement is positive path length through at least one sensitive volume in each required layer. It is not a charge or energy-deposit threshold. HCal bars tile each active layer in the frozen model. Trigger and LYSO bars retain their gaps and staggering.

| Group | Active layers | Active volumes | Geometry identities |
|---|---:|---:|---|
| HCal | 10 | 104 | Top 2, above ECal 4, below ECal 4 |
| Trigger scintillator (TS) | 6 | 72 | Three two-layer stations |
| Tracker | 4 | 4 | Two sensor pairs, including stereo rotations |
| LYSO | 2 | 33 | Two staggered target-module planes |
| ECal | 4 | 4 | Four silicon layers |
| Total | 26 | 217 | One muon must satisfy the specified coincidence |

Tracker numbering should be defined from top to bottom: T1 at z=3309.8880545 mm; T2 at 3303.8880545 mm; T3 at 3209.8880545 mm; T4 at 3203.8880545 mm. A missing tracker requirement means that layer is unrestricted, not vetoed. “At least three of four” is a union of four conditions and must count a muon only once. It includes four-hit muons. “Exactly three” is a different selection and must be labeled explicitly.

## Required distinctions

* “Everything minus ECal” drops the four ECal hit requirements while leaving the installed ECal material in place. The earlier `without_ecal` case physically removed its entire subtree, so it cannot supply this row without modification.
* A layer-AND trigger means every specified layer fires geometrically. A station-OR trigger means any layer in each specified station fires. These alternatives have different rates and must have explicit names.
* Inclusive coincidence rows overlap and must not be added. Exact equipment hit patterns can be tabulated separately if an exclusive partition is wanted.
* HCal-only selection means no requirements on the other installed devices; it does not mean that all other devices must be missed.

## Integration domain

The original `calculate_rate.py` samples endpoints on the first TS layer and the final LYSO layer. That proposal is valid only for rows requiring both anchors. It misses most HCal-only trajectories and cannot be used for those rates by simply changing an output mask.

For every row, choose anchors from its required layers, enlarge the midpoint rectangles to include finite-thickness side entries, and apply all the required layer intersections. The existing bound uses maximum transverse slope divided by the minimum possible anchor separation, D-ha-hb. Proposals can be reused only when their support includes every accepted ray for the target row. Closely spaced station layers have poor endpoint importance sampling; direction-plus-position sampling is preferable for broad single-station rates. Numerical errors should come from independent Sobol scrambles or independent Monte Carlo, and an ordinary Monte Carlo or independent proposal comparison is valuable.

## Energy loss and installed material

The earlier no-roof/concrete estimates use Guan et al.'s modified-Gaisser spectrum normalized by 1.1595376150 to a vertical sea-level intensity of 70 m^-2 s^-1 sr^-1 above p=1 GeV/c. Flux energy is total energy; CSDA table energy is kinetic energy. Three feet of concrete means 91.44 cm at 2.30 g/cm^3, or 210.312 g/cm^2 on the vertical. The slab covers every direction in the model. The 10 MeV residual kinetic-energy floor is a range-table convention, not a readout threshold.

The original 32.7018 g/cm^2 detector column came from 96 narrow all-layer tracks. Broad HCal-trigger trajectories generally do not intersect all the small devices, so this number is not an exact column for them. A new ray-column calculation should retain every installed object intersected before the last required active crossing. Material downstream of the final trigger requirement cannot prevent that requirement from having fired. The 356 material objects in the centered scene include four tracker silicon parents and four active daughters of the same material; count these only once. For stopping, retaining the full parent and excluding the active daughter also preserves the dead border for trajectories that miss the active tracker. ECal placement paths repeat for some boards/glue layers: do not deduplicate objects by path string.

Representing the mixed material column by a polystyrene CSDA range remains an approximation even if ray-by-ray mass columns are calculated exactly. Multiple scattering, straggling, decay, electronics thresholds, detector efficiencies, dead channels, live time, and cosmic shower coincidences remain absent. The CAD support preview is not an additional physical material model in the rate geometry.

## Numerical anchors and validation status

| Aligned all-26 selection | Rate / hour | File |
|---|---:|---|
| Geometric reference, p>1 GeV/c, I=70 cos^2(theta) | 1.5499526316 | `ecal_comparison/centered_only/results.json` |
| Modified-Gaisser + old fixed CSDA column, no roof | 2.1628288829 | `ecal_comparison/centered_only/comparison.json` |
| Same with 3 ft concrete | 1.8420928782 | same |

The geometric and energy-dependent models are different flux integrals; the larger energy-dependent rate includes muons below 1 GeV/c. The centered layout had zero mismatches against the original ROOT shapes over 192 rays and 4,992 individual layer decisions. This cross-check is geometry evidence, not transport validation.

`framework/delivery_status.json`, `framework/README.md`, and `framework/scripts/ldmx_cosmic_config.py` explicitly record that native Geant4/LDMX transport and inter-part overlap validation were not performed. The example native gun is an unexecuted smoke configuration, not a cosmic generator. The general framework's optional six-bar ESA HCal is distinct from the rate geometry's inferred 104-bar, three-station arrangement. Describe the new result as an analytic study based on the frozen, ROOT-checked rate geometry, not as a validated ldmx-sw response simulation.

The frozen geometry provenance records source branch `iss2119-add-hcal-teststand-geometry`, commit `29ee03a8f8fa9eb2ae9c1d29af5e4961ba6c5a0a`, and STEP SHA-256 `ab19afbcafe0d97acc5c7e390ea72bf3d59b37f0edf746f8e20ec150960fa2c2`. Bar counts and registration remain inferred and require hardware or survey confirmation.
