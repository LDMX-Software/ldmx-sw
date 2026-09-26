"""Export the study's aligned ECAL layout as neighboring, loadable GDML files."""

import argparse
import hashlib
import json
import re
import shutil
import xml.etree.ElementTree as ET
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    study = Path(__file__).resolve().parent / "rate_study"
    original = study / "geometry"
    aligned_path = study / "combinations_20260919/inputs/aligned_scene.json"
    aligned = json.loads(aligned_path.read_text())
    offset = json.loads((original / "root_scene.json").read_text())
    shift = aligned["variation"]["ecal_translation_mm"]
    offset_objects = offset["objects"]
    assert len(aligned["objects"]) == len(offset_objects) == 356
    max_difference = 0.0
    # Some repeated GDML daughters share path strings; retain occurrence order.
    for index, obj in enumerate(aligned["objects"]):
        source = offset_objects[index]
        assert obj["path"] == source["path"]
        assert obj["subsystem"] == source["subsystem"]
        assert obj["material"] == source["material"]
        assert len(obj["vertices_mm"]) == len(source["vertices_mm"])
        for vertex_index, vertex in enumerate(obj["vertices_mm"]):
            prior = source["vertices_mm"][vertex_index]
            for axis in range(3):
                displacement = shift[axis] if obj["subsystem"] == "ecal" else 0
                expected = prior[axis] + displacement
                max_difference = max(max_difference, abs(vertex[axis] - expected))
    assert max_difference < 1e-7, "Snapshot differs from an ECAL-only translation"
    output = args.output.resolve()
    if output == study.resolve() or study.resolve() in output.parents:
        raise SystemExit("Choose an output outside the frozen rate_study directory")
    output.mkdir(parents=True, exist_ok=True)
    for path in original.glob("*.gdml"):
        shutil.copy2(path, output / path.name)
    source_text = (original / "detector.gdml").read_text()
    match = re.search(r'<position\b[^>]*name="ecal_pos"[^>]*/>', source_text)
    if match is None:
        raise SystemExit("Expected a unique ECAL placement in the frozen detector GDML")
    position = ET.fromstring(match.group())
    assert position.get("unit") == "mm"
    for index, axis in enumerate("xyz"):
        displacement = shift[index]
        position.set(axis, f"{float(position.get(axis)) + displacement:.15g}")
    replacement = ET.tostring(position, encoding="unicode")
    aligned_text = (
        source_text[: match.start()] + replacement + source_text[match.end() :]
    )
    (output / "detector.gdml").write_text(aligned_text)
    hashes = {
        path.name: hashlib.sha256(path.read_bytes()).hexdigest()
        for path in sorted(output.glob("*.gdml"))
    }
    provenance = {
        "source_detector_sha256": hashlib.sha256(
            (original / "detector.gdml").read_bytes()
        ).hexdigest(),
        "aligned_scene_sha256": hashlib.sha256(aligned_path.read_bytes()).hexdigest(),
        "ecal_translation_mm": shift,
        "checked_objects": len(aligned["objects"]),
        "snapshot_max_abs_difference_mm": max_difference,
        "native_transport_validated": False,
        "files": hashes,
    }
    (output / "alignment_provenance.json").write_text(
        json.dumps(provenance, indent=2) + "\n"
    )
    print(f"Exported {len(hashes)} GDML files to {output}")
    print("ECAL translated; other placements and all material definitions retained.")


if __name__ == "__main__":
    main()
