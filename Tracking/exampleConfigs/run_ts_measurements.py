"""Run the TS -> ldmx::Measurement producer on decoded ZCCM digis.

    just fire Tracking/exampleConfigs/run_ts_measurements.py -- <digi.root> <out.root> [N] [pass]
"""
import sys

sys.argv = [a for a in sys.argv if a != "--"]
from LDMX.Framework import ldmxcfg
from LDMX.Tracking.ts_measurements import TrigScintMeasurementProducer

p = ldmxcfg.Process("tsMeas")
p.input_files = [sys.argv[1]]
p.output_files = [sys.argv[2]]
p.max_events = int(sys.argv[3]) if len(sys.argv) > 3 else 100000

# Register the TrigScintGeometry conditions provider and pin the detector
# (real-data files carry no detector name in their RunHeader).
from LDMX.TrigScint.trigscint_geometry import TrigScintGeometryProvider
from LDMX.TrigScint.ts_daq_map import ts_daq_map_path

TrigScintGeometryProvider.get_instance().set_detector("ldmx-reduced-v3")

tsm = TrigScintMeasurementProducer()
tsm.input_pass = sys.argv[4] if len(sys.argv) > 4 else "unpack"
# DAQ map fixes decode-collection -> geometry-module; z comes from the provider.
tsm.daq_map_file = ts_daq_map_path()
p.sequence = [tsm]
