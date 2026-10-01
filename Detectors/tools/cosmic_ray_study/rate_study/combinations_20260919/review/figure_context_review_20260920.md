# Geometry figure and prior-report context review

No rate calculation was rerun for this review.

## Figure sources

**Left:** render `inputs/aligned_scene.json`, the complete installed-material scene used in the current rate calculation. It contains 356 material objects, including 217 active volumes: 104 HCAL bars, 72 TS bars, four tracker active sensors, 33 LYSO bars, and four ECAL silicon layers. The scene also contains the modeled boards, glue, covers, silicon borders, and detector support pieces. CAD stand mechanics with placeholder material are outside this rate model and should not appear as modeled stopping material. Tracker active daughters overlap their same-material parents in the geometry hierarchy; the rate code counts their material once.

Recommended label: **Installed geometry used in the rate model**. Calling this a full Geant4 simulation would be unsupported.

**Right:** preserve `geometry_acceptance.pdf`, the two-projection image of 32 accepted rays from the earlier rate report. `make_report.py` constructs it from the original `layer_inventory.json` and `accepted_rays.npz`; it uses the original offset-ECAL layout. Its HCAL segments extend beyond the displayed transverse window. Its trajectories are illustrative accepted straight rays, not Geant4 histories or scattering paths. Stacking its two projection panels on the right is reasonable if all labels and the common legend remain legible.

The comparison should explicitly say that the left panel is the current aligned geometry and the right panel preserves the earlier offset-ECAL configuration. A direct comparison of all 356 placements confirms that the only scene change is the complete ECAL translation (+46.55962243, -46.07628914, 0) mm. All other object vertices are unchanged.

Do not substitute `framework/visualization/cosmic_ray_geometry.png`: that older framework illustration has 16 active crossings and shows HCAL CAD objects with unassigned materials. The current rate geometry instead has ten active HCAL layers and requires 26 layers for a golden event. The generic `visualization/overview_closed.png` shows the accepted CAD conversion, not the aligned rate-model material scene.

## Useful earlier-report context

Verified in `framework/report.pdf`, pages 2–3, and the rate report sources:

* Native positive z points downward along CAD negative z after a 180-degree rotation about x. Registration is inferred from CAD features and has not been survey validated.
* The current tracker model has 48 by 78 mm active regions in 50 by 80 by 0.32 mm silicon parents. Sensor pairs are separated by 6 mm; the two stations are separated by 100 mm. CAD candidate rectangles were 40.34 by 100 mm, with 7.3542 mm within-pair separation. The inherited sensitive geometry and physical hardware still require correspondence checks.
* The ECAL model has four 0.3 mm silicon layers, whereas the CAD evidence has six PCB placements and three support plates. A board count is not an active-silicon-layer count.
* The TS bars span only 30 mm in x, while HCAL spans metres. This scale difference explains why the central coincidence is much smaller than the broad HCAL trigger rate.
* Keep original-offset angular summaries and fixed-column historical rates explicitly historical. They must not be attached to the newly aligned figure as if recalculated for it.

## Source identities

* `geometry_acceptance.pdf`: SHA-256 `ce0ef98658aef3af981f03e5005f7099c5ac8c01c23b65c25677f1840507c262`.
* `inputs/aligned_scene.json`: SHA-256 `1fd4445d5c6415efff280e87c141fe2a3ecd7c956d6f5ec40ccbc407dfd95258`.
* `geometry/root_scene.json`: SHA-256 `782b4a133dfadbd489bec17e0409615cf4d9986654fcacf041baa49b80e9f45d`.
