"""check that the standalone processor created a histogram correctly

simple test that hopefully confirms the overall ability of standalone processors
"""

import uproot
from pathlib import Path

file = Path("standalone_histogram.root")
item = "Standalone/event"

if not file.is_file():
    print("Standalone histogram file does not exist")
    exit(2)

with uproot.open(file) as f:
    if item not in f:
        print(f"{item} not in histogram file created by Standalone processor")
        exit(3)

    h = f["Standalone/event"]
    if h.values().sum() != 10.0:
        print(f"Histogram created does not have number of entries expected")
        exit(4)

# exit cleanly and successfully
