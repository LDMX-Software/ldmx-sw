#!/usr/bin/env python3
"""Draw the generated GDML and a separately identified CAD support overlay."""

import argparse
import gzip
import json
import shutil
import subprocess
from pathlib import Path

from visualize_cosmic import is_active, render


HERE = Path(__file__).resolve().parent
GDML = HERE.parents[1] / "data/ldmx-esa-cosmic-cad-v1"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--detectors-only",
        action="store_true",
        help="Omit the CAD supports from the saved display.",
    )
    args = parser.parse_args()
    root = shutil.which("root")
    if not root:
        raise SystemExit("ROOT with GDML support must be on PATH. See README.md.")
    output = HERE / "build"
    output.mkdir(exist_ok=True)
    scene_file = output / "root_scene.json"
    command = [
        root,
        "-l",
        "-b",
        "-q",
        str(HERE / "export_cosmic_scene.C")
        + "("
        + ",".join(
            [
                json.dumps("detector.gdml"),
                json.dumps(str(scene_file)),
                "5027.519993117254",
                "-156.150230102",
                "4000",
                "0",
                "0",
                "-1",
            ]
        )
        + ")",
    ]
    with (output / "root_export.log").open("w") as log:
        subprocess.run(
            command,
            cwd=GDML,
            stdout=log,
            stderr=subprocess.STDOUT,
            check=True,
            timeout=90,
        )
    scene = json.loads(scene_file.read_text())
    for item in scene["objects"]:
        item["active_material"] = is_active(item)
    scene["title"] = "CAD-registered ESA detectors with 104 separate HCal bars"
    scene["caveat"] = (
        "Straight geometric ray, not Geant4 transport. HCal counts and "
        "alignment are provisional."
    )
    if not args.detectors_only:
        overlay = json.loads(
            gzip.decompress((HERE / "cad_preview.json.gz").read_bytes())
        )
        scene["objects"] += overlay["objects"]
    scene["source_gdml"] = "Detectors/data/ldmx-esa-cosmic-cad-v1/detector.gdml"
    result = render(scene, output)
    (output / "scene.json").write_text(json.dumps(scene))
    (output / "display_summary.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
