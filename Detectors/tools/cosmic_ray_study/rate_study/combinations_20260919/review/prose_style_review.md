# Prose and scientific style review

Reviewed the original `cosmic_rate_report.tex` and the conventional PRD manuscript in `continuations/tm_2dgpr_globalprofile_20260717/paper_draft_20260810/main.tex`.

## Recommended presentation

- Use a conventional title, author block, date, abstract, numbered sections, numbered equations, and normal figure/table captions. Computer Modern, booktabs, and restrained hyperlinks match the supplied scientific manuscript. A single-column preprint layout suits the wide rate tables.
- Lead with the aligned geometry and the HCAL trigger definitions. Keep the original displaced layout and raised-assembly studies as a short historical comparison or appendix; their numbers should not compete with the updated baseline in the abstract.
- Suggested title: **Cosmic-muon coincidence rates for the aligned ESA test stand**.
- Suggested opening: "We estimate single-muon coincidence rates in the ESA cosmic test stand with the ECAL aligned to the tracker. The calculation uses the registered active volumes of the stand, a sea-level muon spectrum, and a straight-line continuous-loss approximation. We compare HCAL trigger choices and the rates retained when ECAL or tracker requirements are relaxed."
- Introduce one caveat paragraph where the approximation is defined, then discuss its quantitative consequences in the systematics section. Repeating "not validated" after every result makes a report read defensively and obscures the actual physics.

## Definitions that must be explicit

1. A layer is hit when a track has nonzero path length inside at least one active element of that layer. Hitting every bar is not required.
2. A golden cosmic event hits all 26 active layers of all seven groups.
3. "Without the ECAL requirement" is an inclusive selection: ECAL hits may still be present and its material remains in the geometry. "ECAL missed" is an exclusive selection. "ECAL removed" is a different apparatus and belongs in the prior-layout comparison.
4. "At least three of four tracker layers" counts every qualifying track once. It is the union of the four choices of omitted layer, not their summed rates. If individual leave-one-out rows are included, state that they overlap.
5. A 2-layer, 4-layer, or 10-layer HCAL coincidence is a crossing requirement, not a hardware trigger prediction until thresholds, coincidence windows, efficiencies, and accidental coincidences are supplied.
6. "HCAL only" must say whether it means HCAL requirements alone (inclusive of other hits) or HCAL hit with every other group missed (exclusive).
7. Quote numerical integration errors separately from the flux/material model assumptions. A stable integration does not calibrate the detector response.

## Review checks for the new draft

- All table columns use rates per hour and identify roof/no-roof cases.
- Table captions say whether rows overlap or form a partition.
- For an inclusive required set, dropping requirements cannot lower the rate when the same physical model and integration domain are used.
- The exclusive subsets sum to the corresponding inclusive parent within numerical precision.
- The sampling domain covers the widest HCAL-only configuration; a TS/LYSO anchor can bias an HCAL-only rate even if it worked for golden events.
- Explain the alignment vector and retained original heights once, with the CAD coordinate convention.
- Preserve the original geometry provenance, root checks, range model, roof column, spectrum normalization, finite-thickness test, and earlier layout comparisons.
- Distinguish model-dependent geometric crossing rates from recorded rates in the abstract, results caption, and conclusion; avoid calling an uncalibrated number a DAQ rate.
- Author names were subsequently confirmed by the user: Emrys Peets, Matthew Gignac, and Takumi Britt.

## Updated report review, 19 September 2026

Read the full new `report_body.tex` and applied a prose-only edit. The report now opens with the aligned baseline and clearly labels rates as estimated muon crossings. The tracker relaxation means a missing hit, not a physically absent sensor. The text explains why relaxing tracker requirements leaves the narrow TS/LYSO sample unchanged within the calculation's precision, while leaving room for real efficiency losses. It distinguishes layer AND, station OR, overlapping inclusive selections, and exact tracker multiplicities.

The calculation is described as an analytic geometry and continuous-loss study throughout; no native LDMX transport or calibrated detector-response result is claimed. Historical placement results retain their original fixed-column assumptions. One misleading reference to a “measured” material column was changed to “calculated.” Equations, generated number macros, tables, and figures were left unchanged. Rendered-page checks remain the responsibility of the final PDF build.

## Memory provenance

The quick memory pass confirmed only the existing analytic-rate scope and approximation boundary: `MEMORY.md:102-129`, rollout ID `01a0ab74-27c1-7e70-813f-5d41c8cb183c`. The existing LaTeX report was read directly to verify these points in the current workspace.
