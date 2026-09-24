"""Run the standard TS reco chain + Measurement producer on decoded ZCCM digis.

Takes a ROOT file that already holds decoded ZCCM pads (decodedZCCMPad{1..}) and
runs the standard chain TrigScintRecHitProducer -> TrigScintClusterProducer ->
TrigScintMeasurementProducer, so the measurements come from PE-calibrated
clusters rather than the raw digis.

    just fire Tracking/exampleConfigs/run_ts_measurements.py -- \
        <digi.root> <out.root> [N] [pass]
"""

import sys

sys.argv = [a for a in sys.argv if a != "--"]
from LDMX.Framework import ldmxcfg
from LDMX.Tracking.ts_measurements import (
    TrigScintMeasurementProducer,
    ts_cluster_chain,
)

p = ldmxcfg.Process("tsMeas")
p.input_files = [sys.argv[1]]
p.output_files = [sys.argv[2]]
p.max_events = int(sys.argv[3]) if len(sys.argv) > 3 else 100000
digi_pass = sys.argv[4] if len(sys.argv) > 4 else "unpack"

# Register the TrigScintGeometry conditions provider and pin the detector
# (real-data files carry no detector name in their RunHeader).
from LDMX.TrigScint.trigscint_geometry import TrigScintGeometryProvider

TrigScintGeometryProvider.get_instance().set_detector("ldmx-esa25-v1")

# Standard reco chain reads the decoded pads from the input file's pass.
reco_chain, cluster_collections = ts_cluster_chain(pads=(1, 2, 3), input_pass=digi_pass)

tsm = TrigScintMeasurementProducer()
tsm.input_pass = ""  # clusters are produced in this process
tsm.input_collections = cluster_collections
p.sequence = [*reco_chain, tsm]
