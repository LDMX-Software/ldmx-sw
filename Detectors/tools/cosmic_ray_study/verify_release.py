"""Verify the frozen study files using the published SHA-256 manifest."""

import hashlib
import json
from pathlib import Path


def main():
    study = Path(__file__).resolve().parent / "rate_study"
    manifest = json.loads((study / "delivery_manifest.json").read_text())
    failures = []
    for name, expected in manifest["files"].items():
        path = study / name
        if (
            not path.is_file()
            or hashlib.sha256(path.read_bytes()).hexdigest() != expected
        ):
            failures.append(name)
    if failures:
        raise SystemExit("Missing or changed published files:\n" + "\n".join(failures))
    print(f"Verified {len(manifest['files'])} published study files.")
    print(f"PDF SHA-256: {manifest['sha256']}")


if __name__ == "__main__":
    main()
