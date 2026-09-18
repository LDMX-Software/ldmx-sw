#!/usr/bin/env python3
"""Check schema, packed IDs, bar containment, and bar-to-bar intersections."""

import itertools
import json
import shutil
import subprocess
from pathlib import Path

import numpy as np
from lxml import etree as etree


HERE = Path(__file__).resolve().parent
DATA = HERE.parents[1] / "data"
GDML = DATA / "ldmx-esa-cosmic-cad-v1"
output = HERE / "build"
output.mkdir(exist_ok=True)
bars = json.loads((HERE / "bar_positions.json").read_text())["bars"]
layout = json.loads((HERE / "layout.json").read_text())
expected = {
    s: sum(layer["bars"] for layer in layout["hcal_layers"] if layer["station"] == s)
    for s in ("top", "above_ecal", "below_ecal")
}
assert len({b["copy_number"] for b in bars}) == len(bars) == sum(expected.values())
assert {s: sum(b["station"] == s for b in bars) for s in expected} == expected
for b in bars:
    code = b["copy_number"]
    assert (code >> 24, (code >> 16) & 255, (code >> 8) & 255, code & 255) == (
        1,
        b["section"],
        b["layer"],
        b["strip"],
    )
for a, b in itertools.combinations(bars, 2):
    distance = abs(np.array(a["center_mm"]) - b["center_mm"])
    half_sum = (np.array(a["dimensions_mm"]) + b["dimensions_mm"]) / 2
    assert not np.all(distance < half_sum - 1e-7), (a, b)
entry = etree.parse(str(GDML / "detector.gdml"))
for station in ("top", "above_ecal", "below_ecal"):
    name = "hcal_" + station
    doc = etree.parse(str(GDML / (name + ".gdml")))
    env = doc.find(f"solids/box[@name='{name}_envelope']")
    half = np.array([float(env.get(k)) for k in "xyz"]) / 2
    shapes = {
        s.get("name"): np.array([float(s.get(k)) for k in "xyz"])
        for s in doc.findall("solids/box")
    }
    volumes = {
        v.get("name"): v.find("solidref").get("ref")
        for v in doc.findall("structure/volume")
    }
    for pv in doc.findall(f"structure/volume[@name='{name}']/physvol"):
        center = np.array([float(pv.find("position").get(k)) for k in "xyz"])
        shape = shapes[volumes[pv.find("volumeref").get("ref")]]
        assert np.all(abs(center) + shape / 2 < half + 1e-7), pv.get("name")
    assert (
        entry.find(f"structure/volume[@name='World']/physvol[@name='{name}']/file")
        is not None
    )
schema = etree.XMLSchema(etree.parse(str(DATA / "xsd_files/gdml.xsd")))
for path in GDML.glob("*.gdml"):
    schema.assertValid(etree.parse(str(path)))
root = shutil.which("root")
if not root:
    raise SystemExit(
        "Bar and XSD checks passed, but ROOT is missing; full check is incomplete."
    )
for name in ("detector", "hcal_only"):
    result_path = output / (name + "_root.json")
    macro = (
        str(HERE / "validate_framework.C")
        + "("
        + json.dumps(name + ".gdml")
        + ","
        + json.dumps(str(result_path))
        + ")"
    )
    with (output / (name + "_root.log")).open("w") as log:
        subprocess.run(
            [root, "-l", "-b", "-q", macro],
            cwd=GDML,
            stdout=log,
            stderr=subprocess.STDOUT,
            check=True,
            timeout=90,
        )
scene = json.loads((output / "root_scene.json").read_text())
hcal = [
    o
    for o in scene["objects"]
    if o["subsystem"] == "hcal" and o["material"] == "Scintillator"
]
assert len(hcal) == len(bars), "Rebuild the ROOT display before running check.py"
hcal_crossings = sum(bool(o["ray_intersection"]) for o in hcal)
summary = {
    "root_display_hcal_bars": len(hcal),
    "geometric_hcal_crossings": hcal_crossings,
    "gdml_schema_files": len(list(GDML.glob("*.gdml"))),
    "hcal_bars": len(bars),
    "unique_packed_ids": True,
    "bars_and_covers_inside_station_envelopes": True,
    "bar_to_bar_overlaps": 0,
    "root_imports": 2,
    "whole_detector_overlaps_cleared": False,
    "geant4_transport_tested": False,
    "digitization_or_reconstruction_validated": False,
}
(output / "checks.json").write_text(json.dumps(summary, indent=2) + "\n")
print(json.dumps(summary, indent=2))
