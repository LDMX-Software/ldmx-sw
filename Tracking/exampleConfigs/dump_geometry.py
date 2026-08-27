"""Build the Acts tracking geometry for a detector and dump every surface to CSV.

Writes one row per Acts surface: the surface id used throughout the tracking code
(``volume*1000 + layer*100 + sensor``, see TrackingGeometry.cxx), the surface centre,
and its U/V/W axes. This is the ground truth for anything that has to name a layer --
in particular the tracker DAQ map, which maps (feb, hybrid) onto these ids.

Usage
-----
    denv_workspace="$PWD" denv fire Tracking/exampleConfigs/dump_geometry.py \
        [-- --detector ldmx-reduced-v3] [-- --out geo.csv]
"""

import argparse
import sys

# fire passes '--' through to the script; strip it so argparse sees only flags.
sys.argv = [a for a in sys.argv if a != '--']

parser = argparse.ArgumentParser(f"ldmx fire {sys.argv[0]}")
parser.add_argument(
    "--detector",
    default="ldmx-reduced-v3",
    help="Detector name under Detectors/data (default: ldmx-reduced-v3)",
)
parser.add_argument(
    "--out",
    default="geometry_surfaces.csv",
    help="Output CSV path (default: geometry_surfaces.csv)",
)
arg = parser.parse_args()

from LDMX.Framework import ldmxcfg
from LDMX.Tracking import tracking
from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as TrackGeo

p = ldmxcfg.Process("geodump")
p.max_events = 1
p.log_frequency = 1
# The framework requires an output file even though the CSV is the real product.
p.output_files = ["geodump_events.root"]

TrackGeo.get_instance().set_detector(arg.detector)

# DigitizationProcessor is used purely as a vehicle: its onProcessStart builds the
# geometry conditions object and writes the surface CSV.  No digitization happens --
# there are no sim hits to digitize.
dump = tracking.DigitizationProcessor(instance_name="geodump")
dump.dump_geo_csv = arg.out
dump.hit_collection = "RecoilSimHits"
dump.out_collection = "DumpMeasurements"

p.sequence = [dump]
