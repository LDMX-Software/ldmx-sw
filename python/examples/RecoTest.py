import os,math
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("tracking")

p.libraries.append("libTracking.so")

p.detector = '/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12-dd4hep/detector.xml'

from LDMX.Tracking import tracking_geo
from LDMX.Tracking import tracking_vtx
from LDMX.Tracking import tracking_truthseeder

#Truth seeder  - pions
ts                  = tracking_truthseeder.TruthSeedProcessor()
ts.debug            = False
ts.trk_coll_name    = "RecoilTruthSeeds"
ts.pdgIDs           = [211, -211]
ts.scoring_hits     = "TargetScoringPlaneHits"
ts.z_min            = 4.4
ts.track_id         = -999
ts.p_cut            = 0.
ts.pz_cut           = 0.

#Truth seeder - electrons
ts_ele               = tracking_truthseeder.TruthSeedProcessor()
ts_ele.debug         = False
ts_ele.trk_coll_name = "RecoilTruthSeeds"
ts_ele.pdgIDs        = [11]
ts_ele.scoring_hits  = "TargetScoringPlaneHits"
ts_ele.z_min         = 4.4
ts_ele.track_id      = 1
ts_ele.p_cut         = 0.
ts_ele.pz_cut        = 0.

geo  = tracking_geo.TrackingGeometryMaker()
geo.dumpobj = 0

#propagator tests
#####
geo.ntests = 0#10  #dropped
geo.phi_range   = [-0.99*math.pi, -1*math.pi]  #dropped
geo.theta_range = [0.4 * math.pi, 0.6 * math.pi] #dropped
geo.pt = 4. #dropped
geo.d0sigma = 0.1  #dropped
geo.z0sigma = 0.1  #dropped
geo.steps_file_path = "./straight_propagator_states.root"  #dropped
geo.perigee_location = [-700.,-27.926,0.0] #Generated electrons origin  #dropped

#####

#General
#####
geo.debug = False
geo.propagator_step_size = 1.  #mm
geo.bfield = -1.5  #in T #From looking at the BField map
geo.const_b_field = False
geo.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat"
#geo.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BMapConstant_1_5T.dat"
#####

#CKF Options
geo.hit_collection="RecoilSimHits"
#geo.hit_collection="TaggerSimHits"

#Target location for the CKF extrapolation
geo.seed_coll_name = ts.trk_coll_name
geo.use_extrapolate_location = True  #false not supported anymore
geo.extrapolate_location  = [0.,0.,0.]  #ignored if use_extrapolate_location is False

#Vertexing
vtx = tracking_vtx.VertexProcessor()
vtx.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat"
vtx.debug = False

#p.sequence = [geo, vtx]
p.sequence = [ts,geo,vtx]
#p.sequence = [ts_ele, geo, vtx] #Find electrons


print(p.sequence)

p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]  #single ele

#eN with pions
p.inputFiles = [#os.environ["LDMX_BASE"]+"/data_ldmx/pn_kaonfilter_1M_events_r0690_b462119.root",
    os.environ["LDMX_BASE"]+"/data_ldmx/pn_kaonfilter_1M_events_r0691_b462120.root",
    os.environ["LDMX_BASE"]+"/data_ldmx/pn_kaonfilter_1M_events_r0692_b462121.root",
    os.environ["LDMX_BASE"]+"/data_ldmx/pn_kaonfilter_1M_events_r0693_b462122.root",
]

import glob

p.inputFiles = glob.glob(os.environ["LDMX_BASE"]+"/data_ldmx/pn_kaonfilter_1M*.root")

print(p.inputFiles)

p.outputFiles = ['tracker_test.root']

p.termLogLevel=0
p.maxEvents = 50000
