""" Print change of processor performance between two versions of ldmx from summary.json files"""

import os
import sys
import json

THRESHOLD = 30.0

USAGE_PATH = "vers1.json vers2.json"

if len(sys.argv) != 3:
    print(f"Error, expected two versions. Usage: {sys.argv[0]} {USAGE_PATH}", file=sys.stderr)
    sys.exit(2)

old_vers = sys.argv[1]
new_vers = sys.argv[2]

if (not os.path.isfile(old_vers) or not os.path.isfile(new_vers)):
    print (f"Files not found. Usage: {sys.argv[0]} {USAGE_PATH}", file=sys.stderr)
    sys.exit(2)

def load_json(path):
    """Returns data in processor, total_duration format"""
    with open(path) as f:
        data = json.load(f)
    proc_durs = {}
    for r in data["processors"]:
        proc_durs[r["processor"]] = r["total"]
    return data["metadata"], proc_durs

old_meta, old = load_json(old_vers)
new_meta, new = load_json(new_vers)

for key in ["benchmark_mode", "max_event_count", "CPU_model"]:
    if old_meta.get(key) != new_meta.get(key):
        print(f"Error, {key} differs: {old_meta.get(key)} vs {new_meta.get(key)}", file=sys.stderr)
        sys.exit(2)

print("-" * 86)
print(f"| {'Processor':<30} | {'Old (s)':>10} | {'New (s)' :>10} | {'Change (s)':>11} | {'Change %':>9} |")
print("-" * 86)

regressions = []
for proc in sorted(set(old) | set(new)):
    if proc not in old:
        print(f"| {proc:<30} | {'-':>10} | {new[proc]:>10.4g} | {'-':>11} | {'-':>9} |")
        continue
    if proc not in new:
        print(f"| {proc:<30} | {old[proc]:>10.4g} | {'-':>10} | {'-':>11} | {'-':>9} |")
        continue

    change = new[proc] - old[proc]
    change_pct = change / old[proc] * 100 if old[proc] != 0 else 0.0

    if change_pct > THRESHOLD:
        flag = "REGRESSION"
        regressions.append(proc)
    else:
        flag = ""
    print(f"| {proc:<30} | {old[proc]:>10.4g} | {new[proc]:>10.4g} | {change:>+11.3g} | {change_pct:>+8.1f}% | {flag:<10}")

if regressions:
    print(f"{len(regressions)} processor(s) regressed more than {THRESHOLD}%.")
    sys.exit(1)

    