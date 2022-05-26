import os,math
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("TrackerReco")

p.libraries.append("libTracking.so")

p.detector = '/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12-dd4hep/detector.xml'

from LDMX.Tracking import tracking_geo
from LDMX.Tracking import tracking_vtx
from LDMX.Tracking import tracking_truthseeder

#Truth seeder - electrons
ts_ele               = tracking_truthseeder.TruthSeedProcessor()
ts_ele.debug         = False
ts_ele.trk_coll_name = "RecoilTruthSeeds"
ts_ele.pdgIDs        = [11]
ts_ele.scoring_hits  = "TargetScoringPlaneHits"
ts_ele.z_min         = 4.4
ts_ele.track_id      = 1
ts_ele.p_cut         = 3970. # In MeV
ts_ele.pz_cut        = 0.


uSmearing = 0.02
vSmearing = 0.1

geo  = tracking_geo.TrackingGeometryMaker("Recoil_TrackFinder")
geo.dumpobj = 0

#propagator tests
#####
geo.ntests = 0#10  #dropped
geo.phi_range   = [-0.99*math.pi, -1*math.pi]  #dropped
geo.theta_range = [0.4 * math.pi, 0.6 * math.pi] #dropped
geo.pt = 4. #dropped
geo.d0sigma = 0.1  #dropped
geo.z0sigma = 0.1  #dropped
geo.steps_file_path = "./recoil_evt_display.root"  #dropped
geo.perigee_location = [-700.,-27.926,0.0] #Generated electrons origin  #dropped

#####

#General
####
geo.debug = False
geo.propagator_step_size = 1.  #mm
geo.propagator_maxSteps = 2000
geo.bfield = -0.75  #in T #From looking at the BField map
geo.const_b_field = False
geo.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat"
#geo.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BMapConstant_1_5T.dat"
#####

#CKF Options
geo.hit_collection="RecoilSimHits"
geo.out_trk_collection = "RecoilTracks"

#Target location for the CKF extrapolation
geo.seed_coll_name = "RecoilTruthSeeds"
geo.use_extrapolate_location = True  #false not supported anymore
geo.extrapolate_location  = [0.,0.,0.]  #ignored if use_extrapolate_location is False

#smear the hits used for finding/fitting
geo.do_smearing = False;
geo.sigma_u = uSmearing
geo.sigma_v = vSmearing
geo.trackID = 1
geo.pdgID = 11



geo_tagger  = tracking_geo.TrackingGeometryMaker("Tagger_TrackFinder")
geo_tagger.dumpobj = 0

#General
####
geo_tagger.debug = False
geo_tagger.propagator_step_size = 2.  #mm
geo_tagger.bfield = -1.5  #in T #From looking at the BField map
geo_tagger.const_b_field = False
geo_tagger.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat"
#geo.bfieldMap_ = "/Users/pbutti/sw/data_ldmx/BMapConstant_1_5T.dat"
#####

#propagator tests
#####
geo_tagger.ntests = 0#10  #dropped
geo_tagger.phi_range   = [-0.99*math.pi, -1*math.pi]  #dropped
geo_tagger.theta_range = [0.4 * math.pi, 0.6 * math.pi] #dropped
geo_tagger.pt = 4. #dropped
geo_tagger.d0sigma = 0.1  #dropped
geo_tagger.z0sigma = 0.1  #dropped
geo_tagger.steps_file_path = "./straight_propagator_states.root"  #dropped
geo_tagger.perigee_location = [-700.,-27.926,0.0] #Generated electrons origin  #dropped

#CKF Options
geo_tagger.hit_collection="TaggerSimHits"

#Target location for the CKF extrapolation
geo_tagger.seed_coll_name = "TaggerTruthSeeds"
geo_tagger.use_extrapolate_location = True  #false not supported anymore
geo_tagger.extrapolate_location  = [0.,0.,0.]  #ignored if use_extrapolate_location is False
geo_tagger.out_trk_collection = "TaggerTracks"

#smear the hits used for finding/fitting
geo_tagger.do_smearing = False
geo_tagger.sigma_u = uSmearing  #mm
geo_tagger.sigma_v = vSmearing    #mm
geo_tagger.trackID = 1
geo_tagger.pdgID = 11


p.sequence = [ts_ele, geo,geo_tagger]

print(p.sequence)

#p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]  #single ele
p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/single_e/mc_v12-4GeV-1e-inclusive_run10001_t1636673834.root",
                os.environ["LDMX_BASE"]+"/data_ldmx/single_e/mc_v12-4GeV-1e-inclusive_run10001_t1636673835.root",
                os.environ["LDMX_BASE"]+"/data_ldmx/single_e/mc_v12-4GeV-1e-inclusive_run10001_t1636674259.root",
                os.environ["LDMX_BASE"]+"/data_ldmx/single_e/mc_v12-4GeV-1e-inclusive_run10002_t1636673832.root",
                os.environ["LDMX_BASE"]+"/data_ldmx/single_e/mc_v12-4GeV-1e-inclusive_run10002_t1636673834.root",
                os.environ["LDMX_BASE"]+"/data_ldmx/single_e/mc_v12-4GeV-1e-inclusive_run10002_t1636673863.root",
                os.environ["LDMX_BASE"]+"/data_ldmx/single_e/mc_v12-4GeV-1e-inclusive_run10003_t1636673742.root",
                ]


print(p.inputFiles)

p.keep = [
    "drop .*SimHits.*", #drop all sim hits
    "drop .*Ecal.*", #drop all ecal (Digis are not removed)
#    "drop .*Magnet*",
    "drop .*Hcal.*",
    "drop .*Scoring.*",
    "drop .*SimParticles.*",
    "drop .*TriggerPad.*",
    "drop .*trig.*"
    ]


p.outputFiles = ['single_ele_tagger.root']

p.termLogLevel=0
p.maxEvents = 10000

