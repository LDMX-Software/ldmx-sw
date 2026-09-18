# CAD-registered ESA cosmic stand

Start with `detector.gdml` for the combined detector, or `hcal_only.gdml` for its
three segmented HCal stations. Coordinates are the original STEP frame in mm;
+Z points upward, so a downward muon travels along -Z.

These files are generated. The [viewer README](../../tools/cosmic_viewer/README.md)
contains example images, setup commands, the source of each dimension, and the
editing instructions. In particular, the 12/8/12 HCal bar counts are inferred from
CAD body widths and need hardware confirmation. The legacy readout-position model
must not be used as a position lookup for this layout.

The detailed CAD supports are included only in the browser overlay, not in this
Geant4 geometry. Detector alignment and whole-detector overlaps remain under review.
