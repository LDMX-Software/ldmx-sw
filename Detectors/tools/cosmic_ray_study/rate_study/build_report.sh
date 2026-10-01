#!/usr/bin/env bash
set -euo pipefail
rate_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$rate_dir"
rate_python="${PYTHON:-python3}"
"$rate_python" combinations_20260919/calculate_combinations.py --power 17 --replicates 8
"$rate_python" combinations_20260919/calculate_hcal_triggers.py --power 17 --replicates 8
"$rate_python" combinations_20260919/validate_numerics.py
if [[ "${RUN_ROOT_VALIDATION:-0}" == "1" ]]; then
  "$rate_python" combinations_20260919/validate_ray_geometry.py
  "$rate_python" combinations_20260919/validate_station_angles.py
fi
"$rate_python" combinations_20260919/make_assets.py
tectonic -X compile cosmic_rate_report.tex --keep-logs
"$rate_python" combinations_20260919/package_manifest.py
