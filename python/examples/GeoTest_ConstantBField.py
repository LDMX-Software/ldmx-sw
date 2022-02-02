import os,math
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("tracking")

p.libraries.append("libTracking.so")

p.detector = '/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12-dd4hep/detector.xml'

from LDMX.Tracking import tracking_geo

geo  = tracking_geo.TrackingGeometryMaker()
geo.dumpobj = 0


#propagator tests
#####
geo.ntests = 0#10
geo.phi_range   = [-0.99*math.pi, -1*math.pi]
geo.theta_range = [0.4 * math.pi, 0.6 * math.pi]
geo.pt = 4.
geo.d0sigma = 0.1
geo.z0sigma = 0.1
geo.steps_file_path = "./straight_propagator_states.root"
#####

#General
#####
geo.debug = False
geo.propagator_step_size = 2.  #mm
geo.bfield = -1.5  #in T #From looking at the BField map
geo.const_b_field = False
geo.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat"
#geo.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BMapConstant_1_5T.dat"
#####

#Detector specific tracking -> It has to be already rotated in the acts frame
#geo.perigee_location = [-700.,-27.926,0.0] #Generated electrons origin  
geo.perigee_location = [ 0.0, 0.0, 0.0 ]  #Target location

#
geo.hit_collection="RecoilSimHits"
#geo.hit_collection="TaggerSimHits"

p.sequence = [geo]

print(p.sequence)

p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]
p.outputFiles = ['tracker_test.root']

p.termLogLevel=0
p.maxEvents = 10000
