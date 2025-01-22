# Example of jobOption to run tracking on input simulated files in ldmx-sw
# For detailed description of the various configurations, check the .py module files inside
# Tracking/python

import os,math
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("sim")

# Load the tracking module
from LDMX.Tracking import tracking

# This has to stay after defining the "TrackerReco" Process in order to load the geometry
# From the conditions
from LDMX.Tracking import geo


# Truth seeder 
# Runs truth tracking producing tracks from target scoring plane hits for Recoil
# and generated electros for Tagger.
# Truth tracks can be used for assessing tracking performance or using as seeds

truth_tracking           = tracking.TruthSeedProcessor()
truth_tracking.debug             = False
truth_tracking.pdgIDs            = [11]
truth_tracking.scoring_hits      = "TargetScoringPlaneHits"
truth_tracking.z_min             = 0.
truth_tracking.track_id          = -1
truth_tracking.p_cut             = 0.05 # In MeV
truth_tracking.pz_cut            = 0.03
truth_tracking.p_cutEcal         = 0. # In MeV


# These smearing quantities are default. We expect around 6um hit resolution in bending plane
# v-smearing is actually not used as 1D measurements are used for tracking. These smearing parameters
# are fed to the digitization producer.
uSmearing = 0.006       #mm
vSmearing = 0.000001    #mm


# Smearing Processor - Tagger
# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to reduce combinatorics
digiTagger = tracking.DigitizationProcessor("DigitizationProcessor")
digiTagger.hit_collection = "TaggerSimHits"
digiTagger.out_collection = "DigiTaggerSimHits"
digiTagger.merge_hits = True
digiTagger.sigma_u = uSmearing
digiTagger.sigma_v = vSmearing

# Smearing Processor - Recoil
digiRecoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digiRecoil.hit_collection = "RecoilSimHits"
digiRecoil.out_collection = "DigiRecoilSimHits"
digiRecoil.merge_hits = True
digiRecoil.sigma_u = uSmearing
digiRecoil.sigma_v = vSmearing

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at the track
# parameters and the impact parameters at the target or generation point. For the tagger one should look
# for compatibility with the beam orbit / beam spot
seederTagger = tracking.SeedFinderProcessor("SeedTagger")
seederTagger.input_hits_collection =  digiTagger.out_collection
#seederTagger.perigee_location = [0.,0.,0.]
seederTagger.out_seed_collection = "TaggerRecoSeeds"
#seederTagger.pmin  = 2.
seederTagger.pmax  = 9.
seederTagger.d0min = -60.
seederTagger.d0max = 60.
#seederTagger.d0max =  60. #60. # 0

#Seed finder processor - Recoil
seederRecoil = tracking.SeedFinderProcessor("SeedRecoil")
seederRecoil.perigee_location = [0.,0.,0.]
seederRecoil.input_hits_collection =  digiRecoil.out_collection
seederRecoil.out_seed_collection = "RecoilRecoSeeds"
#seederRecoil.bfield = 1.5
seederRecoil.pmin  = 0.49092868011123036
seederRecoil.pmax  =  98.13998986979094 #4
seederRecoil.d0min =  -27.14670790136963 #-60. #-0.5
seederRecoil.d0max = 30.34555944396442 #60. #0.5
seederRecoil.z0max = 50.601628971164075 #60. #10
seederRecoil.thetacut = 1.9359243746789738
seederRecoil.phicut =  0.4895040472097839



# Producer for running the CKF track finding starting from the found seeds.
tracking_tagger  = tracking.CKFProcessor("Tagger_TrackFinder")
tracking_tagger.dumpobj = False
tracking_tagger.debug = False
tracking_tagger.propagator_step_size = 1000.  #mm
tracking_tagger.bfield = -1.5  #in T #From looking at the BField map
tracking_tagger.const_b_field = False

#Target location for the CKF extrapolation
tracking_tagger.seed_coll_name = "TaggerRecoSeeds" #seederTagger.out_seed_collection #"TaggerTruthSeeds" #
tracking_tagger.out_trk_collection = "TaggerTracks"

#smear the hits used for finding/fitting
tracking_tagger.trackID = -1 #1
tracking_tagger.pdgID = -9999 #11
tracking_tagger.measurement_collection = digiTagger.out_collection
tracking_tagger.min_hits = 5
tracking_tagger.outlier_pval_ = 19.902973014794874

#CKF Options
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.dumpobj = False
tracking_recoil.debug = False
tracking_recoil.propagator_step_size = 1000.  #mm
tracking_recoil.bfield = -1.5  #in T #From looking at the BField map
tracking_recoil.const_b_field = False
tracking_recoil.taggerTracking = False

#Target location for the CKF extrapolation
#tracking_recoil.seed_coll_name = seederRecoil.out_seed_collection
tracking_recoil.seed_coll_name = "RecoilRecoSeeds"
tracking_recoil.out_trk_collection = "RecoilTracks"

#smear the hits used for finding/fitting
tracking_recoil.trackID = -1 #1
tracking_recoil.pdgID = -9999 #11
tracking_recoil.measurement_collection = digiRecoil.out_collection
tracking_recoil.min_hits = 5

GSF_tagger = tracking.GSFProcessor("Tagger_GSF")
GSF_tagger.trackCollection = "TaggerTracksClean"
GSF_tagger.measCollection  = "DigiTaggerSimHits"
GSF_tagger.out_trk_collection = "GSFTagger"
GSF_tagger.taggerTracking = True
GSF_tagger.debug = False

GSF_recoil = tracking.GSFProcessor("Recoil_GSF")
GSF_recoil.trackCollection = "RecoilTracksClean"
GSF_recoil.measCollection  = "DigiRecoilSimHits"
GSF_recoil.out_trk_collection = "GSFRecoil"
GSF_recoil.taggerTracking = False
GSF_recoil.debug = True

greedy_solver_tagger = tracking.GreedyAmbiguitySolver("GreedySolverTagger")
greedy_solver_tagger.nMeasurementsMin = 5
greedy_solver_tagger.maximumSharedHits = 2
greedy_solver_tagger.out_trk_collection = "TaggerTracksClean"
greedy_solver_tagger.trackCollection = "TaggerTracks"
greedy_solver_tagger.measCollection = "DigiTaggerSimHits"

greedy_solver_recoil = tracking.GreedyAmbiguitySolver("GreedySolverRecoil")
greedy_solver_recoil.nMeasurementsMin = 5
greedy_solver_recoil.maximumSharedHits = 2
greedy_solver_recoil.out_trk_collection = "RecoilTracksClean"
greedy_solver_recoil.trackCollection = "RecoilTracks"
greedy_solver_recoil.measCollection = "DigiRecoilSimHits"

'''

score_solver_tagger = tracking.ScoreBasedAmbiguitySolver("ScoreSolverTagger")
score_solver_tagger.addDetectorConfig(volumes = [2,3], hitsScoreWeight = 0,
                                      holesScoreWeight = -1, outliersScoreWeight = -1,
                                      otherScoreWeight = -1, minHits = 5, maxHits = 14, maxHoles = 1,
                                      maxOutliers = 2, maxSharedHits = 2, sharedHitsFlag = True, 
                                      detectorId = 0, factorHits = [1.0], factorHoles = [1.0])

#score_solver_tagger.addDetectorConfig(volumes = [4], hitsScoreWeight = 1,
#                                      holesScoreWeight = 1, outliersScoreWeight = 1,
#                                      otherScoreWeight = 1, minHits = 1, maxHits = 1, maxHoles = 1,
#                                      maxOutliers = 1, maxSharedHits = 1, sharedHitsFlag = True, 
#                                    detectorId = 1, factorHits = [1.0], factorHoles = [1.0])

score_solver_tagger.minScore = 1.5
score_solver_tagger.minScoreSharedTracks = 1.5
score_solver_tagger.out_trk_collection = "TaggerScoreClean"
score_solver_tagger.trackCollection = "TaggerTracks"
score_solver_tagger.measCollection = "DigiTaggerSimHits"
score_solver_tagger.verbose = True
score_solver_tagger.etaMin = -100.
score_solver_tagger.etaMax = 100.
'''
  

from LDMX.Tracking import dqm
digi_dqm = dqm.TrackerDigiDQM()
tracking_dqm = dqm.TrackingRecoDQM()

seed_recoil_dqm = dqm.TrackingRecoDQM("SeedRecoilDQM")
seed_recoil_dqm.track_collection = seederRecoil.out_seed_collection
seed_recoil_dqm.truth_collection = "RecoilTruthSeeds"
seed_recoil_dqm.title = ""
seed_recoil_dqm.buildHistograms()


recoil_dqm = dqm.TrackingRecoDQM("RecoilDQM")
recoil_dqm.track_collection = "GSFRecoil"
recoil_dqm.truth_collection = "RecoilTruthTracks"
recoil_dqm.title = ""
recoil_dqm.buildHistograms()

#recoil_dqm_gsf = dqm.TrackingRecoDQM("RecoilDQMGSF")
#recoil_dqm_gsf.track_collection = "RecoilTracksClean"
#recoil_dqm_gsf.truth_collection = "RecoilTruthTracks"
#recoil_dqm_gsf.title = ""
#recoil_dqm_gsf.buildHistograms()


seed_tagger_dqm = dqm.TrackingRecoDQM("SeedTaggerDQM")
seed_tagger_dqm.track_collection = seederTagger.out_seed_collection
seed_tagger_dqm.truth_collection = "TaggerTruthSeeds"
seed_tagger_dqm.title = ""
seed_tagger_dqm.buildHistograms()


tagger_dqm = dqm.TrackingRecoDQM("TaggerDQM")
tagger_dqm.track_collection = tracking_tagger.out_trk_collection
tagger_dqm.truth_collection = "TaggerTruthTracks"
tagger_dqm.title = ""
tagger_dqm.buildHistograms()


# This sequence runs the digitization in the tagger and recoil
# Then the truth tracking to have TruthTracks in the final state
# the nominal seeding is ran on the tagger and recoil
# Track finding is then raun in the tagger and in the recoil
# Finally two dqm examples are run in the recoil tracks and using the seed tracks

p.sequence   = [digiTagger, digiRecoil,
                truth_tracking,
                seederTagger, seederRecoil,
                tracking_tagger, tracking_recoil, greedy_solver_tagger, greedy_solver_recoil,
                recoil_dqm, seed_recoil_dqm, seed_tagger_dqm, tagger_dqm]

# The input file to be added.
# for now, we just take the first argument on the command line
import sys
p.inputFiles = [sys.argv[1]]
outputNameString= str(sys.argv[2]) #sample identifier
outDir= str(sys.argv[3])    #sample identifier
outname=outDir+'/'+outputNameString #+".root" #+'_withTracking.root'#+".root"


# Example about how to drop some of the collections in the output file.
#p.keep = [
#    #    "drop .*SimHits.*", #drop all sim hits
#    "drop .*Ecal.*", #drop all ecal (Digis are not removed)
#    #    "drop .*Magnet*",
#    "drop .*Hcal.*",
#    "drop .*Scoring.*",
#    "drop .*SimParticles.*",
#    "drop .*TriggerPad.*",
#    "drop .*trig.*"
#]

# Output name
#   just append '_withTracking' to the name of the input file
from pathlib import Path
input_filepath = Path(p.inputFiles[0])
p.outputFiles = [outname]

# lower log level so 'info' and above messages can be printed
p.termLogLevel=4

# Number of events
p.maxEvents = 2000
# Where to store DQM plots
p.histogramFile = outDir+'_hists/'+outputNameString #' #outDir+'_hists/'+outputNameString #+"_hists.root"