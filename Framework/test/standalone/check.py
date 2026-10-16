"""check that the standalone processors created their histograms correctly

simple test that hopefully confirms the overall ability of standalone processors
"""

from pathlib import Path

import uproot


file = Path("standalone_histogram.root")
items = ["Standalone/event", "StandaloneG4/action_type"]

if not file.is_file():
    print("Standalone histogram file does not exist")
    exit(2)

with uproot.open(file) as f:
    for item in items:
        if item not in f:
            print(f"{item} not in histogram file created by standalone processors")
            exit(3)

        h = f[item]
        if h.values().sum() != 10.0:
            print(f"{item} does not have number of entries expected")
            exit(4)

# exit cleanly and successfully
