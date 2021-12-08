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
geo.ntests = 0 #10
geo.phi_range   = [-0.99*math.pi, -1*math.pi]
geo.theta_range = [0.4 * math.pi, 0.6 * math.pi]
geo.pt = 4.
geo.d0sigma = 0.
geo.z0sigma = 0.
geo.bfield = -1.441  #From looking at the BField map
geo.propagator_step_size = 250.  #mm
geo.perigee_location = [-0.,0.,0.]


p.sequence = [geo]

print(p.sequence)

p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]
p.outputFiles = ['tracker_test.root']

p.termLogLevel=0

p.maxEvents = 5
