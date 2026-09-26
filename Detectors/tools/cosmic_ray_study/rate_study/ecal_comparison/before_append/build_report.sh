#!/usr/bin/env bash
set -euo pipefail
rate_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$rate_dir"
rate_python="${PYTHON:-python3}"
"$rate_python" calculate_rate.py --scene geometry/root_scene.json
(
  cd geometry
  root -l -b -q '../check_rays.C("detector.gdml","..")' > ../root_check.log 2>&1
)
"$rate_python" shielding.py
"$rate_python" shift_up_200mm/calculate_shift.py
"$rate_python" shift_up_200mm/calculate_shift.py --center-ecal
"$rate_python" shift_up_200mm/make_section.py
"$rate_python" make_report.py
tectonic -X compile cosmic_rate_report.tex --keep-logs
"$rate_python" manifest.py
