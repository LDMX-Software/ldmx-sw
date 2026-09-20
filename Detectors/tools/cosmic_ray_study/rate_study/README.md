# ESA cosmic-muon trigger and coincidence rates

Presentation revised 20 September 2026; rate calculations frozen 19 September
2026. Authors: Emrys Peets, Matthew Gignac, Takumi Britt.

Read **cosmic_rate_report.pdf** for the aligned-ECAL study, HCAL triggers, golden
cosmic events, detector-combination tables, earlier placement studies and
scientific limitations. The original report is preserved in
`combinations_20260919/previous_report/`.

Figure 1 compares the full aligned rate-model geometry with the earlier report's
accepted rays in the offset layout. Figure 2 combines the aligned active-layer
views and retained-rate plot; bold cardinal-red labels give both hourly rates.

The complete source, result-table definitions, reproduction commands and
validation guide are in **combinations_20260919/README.md**.

Run `PYTHON=/path/to/python3 bash build_report.sh` from this directory.
The build entry point now rebuilds the 19 September study. The earlier scripts
remain available for provenance; their original build entry point is preserved
with the previous report. Do not run the legacy `make_report.py` to build the
updated study: use the new build entry point or `combinations_20260919/make_assets.py`.

Results are conditional straight-ray/CSDA muon-crossing estimates before
detector response. SHA256SUMS and delivery_manifest.json identify the files.
