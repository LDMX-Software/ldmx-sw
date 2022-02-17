import os
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("tracking")
p.libraries.append("libTracking.so")

p.detector = '/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12-dd4hep/detector.xml'

from LDMX.Tracking import tracking_vtx

vtx = tracking_vtx.VertexProcessor()
vtx.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat"
vtx.debug = True

p.sequence = [vtx]
print(p.sequence)

p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]
p.outputFiles = ['vertex_test.root']

p.termLogLevel=0
p.maxEvents = 10


