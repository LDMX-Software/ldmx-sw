import os,math
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("tracking")

p.libraries.append("libTracking.so")

p.detector = '/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12-dd4hep/detector.xml'

from LDMX.Tracking import tracking_geo

geo  = tracking_geo.TrackingGeometryMaker()
geo.dumpobj = 0
geo.steps_file_path = "./straight_propagator_states.root"

#propagator tests
geo.ntests = 100
geo.phi_range   = [-0.7 * math.pi, -1.3 * math.pi]
geo.theta_range = [0.2 * math.pi, 0.8 * math.pi]
geo.bfield = 1.15
geo.propagator_step_size = 0.1
geo.perigee_location = [-300.,0.,0.]

p.sequence = [geo]

print(p.sequence)

p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]
p.outputFiles = ['tracker_test.root']

p.termLogLevel=0

p.maxEvents = 1
