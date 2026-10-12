"""TS TrigScintMeasurements from an EVENT-BUILT ROOT (matched with tracker).

Decodes the EventBuilder's per-event `ts` byte branch (collection 'ts', pass
'builder') with the ZCCM decoder, then runs the standard reconstruction chain
(TrigScintRecHitProducer -> TrigScintClusterProducer), and finally the
geometry-aware TrigScintMeasurementProducer on the clusters.  Reads the SAME
event-built file as the tracker decode, so the output is event-aligned (match by
entry index).

The reco chain (LDMX.Tracking.ts_measurements.ts_cluster_chain) uses the standard
TS rechit/cluster producers, so the measurements are built from
pedestal-subtracted, PE-calibrated clusters -- not the raw ZCCM digis. (The CI
sample esa_vst/config.py uses the TestBeamHit/TestBeamCluster producers instead;
those are leftovers from an old CERN test beam and are not used here.)

    denv fire Tracking/exampleConfigs/decode_ts_from_evb.py -- \
        <evb.root> <out.root> [max_events]
"""

import os
import sys

sys.argv = [a for a in sys.argv if a != "--"]
inp = sys.argv[1]
out = sys.argv[2]
maxev = int(sys.argv[3]) if len(sys.argv) > 3 else -1

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("tsevb")
p.input_files = [inp]
p.output_files = [out]
if maxev > 0:
    p.max_events = maxev

from LDMX.TrigScint.zccm_format import ZCCMDecoder

here = os.path.dirname(os.path.abspath(__file__))
cmap = os.path.join(
    here, "..", "..", "TrigScint", "data", "channelMap_4modules_14lanes.txt"
)
cmap = os.path.abspath(cmap)
mmap = cmap.replace("channelMap", "moduleMap")

# Stage 1: decode the event-built `ts_builder` byte branch -> decodedZCCMPad{1..}.
dec = ZCCMDecoder(channel_map_file=cmap)
dec.module_map_file = mmap
dec.input_collection = "ts"  # the event-built `ts_builder` byte branch
dec.input_pass_name = "builder"
dec.output_collection = dec.output_collection + "Pad"  # -> decodedZCCMPad{1..}
dec.number_channels = 84
dec.number_time_samples = 30
dec.is_real_data = True

# Stage 2: standard reco chain (TrigScintRecHit -> TrigScintCluster) for the
# three plastic pads.
from LDMX.TrigScint.trigscint_geometry import TrigScintGeometryProvider
from LDMX.Tracking.ts_measurements import (
    TrigScintMeasurementProducer,
    ts_cluster_chain,
)

reco_chain, cluster_collections = ts_cluster_chain(pads=(1, 2, 3))

# Stage 3: clusters -> geometry-aware ldmx::Measurement.
TrigScintGeometryProvider.get_instance().set_detector("ldmx-esa25-v1")
tsm = TrigScintMeasurementProducer()
tsm.input_pass = ""  # any pass (produced in this same process)
tsm.input_collections = cluster_collections

p.sequence = [dec, *reco_chain, tsm]
p.logger.term_level = 2
