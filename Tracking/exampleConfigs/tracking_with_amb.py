# Example of jobOption to run tracking on input simulated files in ldmx-sw
# For detailed description of the various configurations, check the .py module files
# inside
# Tracking/python

import os
import math
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("sim")

# Load the tracking module
from LDMX.Tracking import tracking

# This has to stay after defining the "TrackerReco" Process in order to load the
# geometry
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


# These smearing quantities are default. We expect around 6um hit resolution in bending
# plane
# v-smearing is actually not used as 1D measurements are used for tracking. These
# smearing parameters
# are fed to the digitization producer.
u_smearing = 0.006       #mm
v_smearing = 0.000001    #mm


# Smearing Processor - Tagger
# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to
# reduce combinatorics
digi_tagger = tracking.DigitizationProcessor("DigitizationProcessor")
digi_tagger.hit_collection = "TaggerSimHits"
digi_tagger.out_collection = "DigiTaggerSimHits"
digi_tagger.merge_hits = True
digi_tagger.sigma_u = u_smearing
digi_tagger.sigma_v = v_smearing

# Smearing Processor - Recoil
digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digi_recoil.hit_collection = "RecoilSimHits"
digi_recoil.out_collection = "DigiRecoilSimHits"
digi_recoil.merge_hits = True
digi_recoil.sigma_u = u_smearing
digi_recoil.sigma_v = v_smearing

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting
# them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at
# the track
# parameters and the impact parameters at the target or generation point. For the tagger
# one should look
# for compatibility with the beam orbit / beam spot
seeder_tagger = tracking.SeedFinderProcessor("SeedTagger")
seeder_tagger.input_hits_collection =  digi_tagger.out_collection
#seeder_tagger.perigee_location = [0.,0.,0.]
seeder_tagger.out_seed_collection = "TaggerRecoSeeds"
seeder_tagger.pmin  = 0.02743004128947902
seeder_tagger.pmax  =  63.03941963609926 #4
seeder_tagger.d0min =  -36.90755883944579 #-60. #-0.5
seeder_tagger.d0max = 31.512252170025366 #60. #0.5
seeder_tagger.z0max = 54.622547563516235 #60. #10
seeder_tagger.thetacut = 0.2622650393957737
seeder_tagger.phicut =  0.8370135051959274

#Seed finder processor - Recoil
seeder_recoil = tracking.SeedFinderProcessor("SeedRecoil")
seeder_recoil.perigee_location = [0.,0.,0.]
seeder_recoil.input_hits_collection =  digi_recoil.out_collection
seeder_recoil.out_seed_collection = "RecoilRecoSeeds"
seeder_recoil.pmin  =   0.04311669558561525
seeder_recoil.pmax  =  819.0346117063144
seeder_recoil.d0min =  -40.20866044915365
seeder_recoil.d0max = 36.47833845632701
seeder_recoil.z0max = 40.517081118987285
seeder_recoil.thetacut =  1.4783718595311992
seeder_recoil.phicut =  1.5786029498528924



# Producer for running the CKF track finding starting from the found seeds.
tracking_tagger  = tracking.CKFProcessor("Tagger_TrackFinder")
tracking_tagger.dumpobj = False
tracking_tagger.debug = False
tracking_tagger.propagator_step_size = 1000.  #mm
tracking_tagger.const_b_field = False
tracking_tagger.seed_coll_name = "TaggerRecoSeeds" #seeder_tagger.out_seed_collection #"TaggerTruthSeeds" #
tracking_tagger.out_trk_collection = "TaggerTracks"
tracking_tagger.measurement_collection = digi_tagger.out_collection
tracking_tagger.min_hits = 5
tracking_tagger.outlier_pval_ = 16.501226781496662

#CKF Options
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.dumpobj = False
tracking_recoil.debug = False
tracking_recoil.propagator_step_size = 1000.  #mm
tracking_recoil.bfield = -1.5  #in T #From looking at the BField map
tracking_recoil.const_b_field = False
tracking_recoil.taggerTracking = False
tracking_recoil.seed_coll_name = "RecoilRecoSeeds"
tracking_recoil.out_trk_collection = "RecoilTracks"
tracking_recoil.measurement_collection = digi_recoil.out_collection
tracking_recoil.min_hits = 8
tracking_recoil.outlier_pval_ =  22.165497985508754


GSF_tagger = tracking.GSFProcessor("Tagger_GSF")
GSF_tagger.track_collection = "TaggerTracksClean"
GSF_tagger.meas_collection  = "DigiTaggerSimHits"
GSF_tagger.out_trk_collection = "GSFTagger"
GSF_tagger.taggerTracking = True
GSF_tagger.debug = False

GSF_recoil = tracking.GSFProcessor("Recoil_GSF")
GSF_recoil.track_collection = "RecoilTracksClean"
GSF_recoil.meas_collection  = "DigiRecoilSimHits"
GSF_recoil.out_trk_collection = "GSFRecoil"
GSF_recoil.taggerTracking = False
GSF_recoil.debug = True

greedy_solver_tagger = tracking.GreedyAmbiguitySolver("GreedySolverTagger")
greedy_solver_tagger.nMeasurementsMin = 5
greedy_solver_tagger.maximumSharedHits = 2
greedy_solver_tagger.out_trk_collection = "TaggerTracksClean"
greedy_solver_tagger.track_collection = "TaggerTracks"
greedy_solver_tagger.meas_collection = "DigiTaggerSimHits"

greedy_solver_recoil = tracking.GreedyAmbiguitySolver("GreedySolverRecoil")
greedy_solver_recoil.nMeasurementsMin = 5
greedy_solver_recoil.maximumSharedHits = 2
greedy_solver_recoil.out_trk_collection = "RecoilTracksClean"
greedy_solver_recoil.track_collection = "RecoilTracks"
greedy_solver_recoil.meas_collection = "DigiRecoilSimHits"

from LDMX.Tracking import dqm
digi_dqm = dqm.TrackerDigiDQM()
tracking_dqm = dqm.TrackingRecoDQM()

seed_recoil_dqm = dqm.TrackingRecoDQM("SeedRecoilDQM")
seed_recoil_dqm.track_collection = seeder_recoil.out_seed_collection
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
seed_tagger_dqm.track_collection = seeder_tagger.out_seed_collection
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

p.sequence   = [digi_tagger, digi_recoil,
                truth_tracking,
                seeder_tagger, seeder_recoil,
                tracking_tagger, tracking_recoil, greedy_solver_tagger, greedy_solver_recoil,
                recoil_dqm, seed_recoil_dqm, seed_tagger_dqm, tagger_dqm]

# The input file to be added.
# for now, we just take the first argument on the command line
import sys
p.input_files = [sys.argv[1]]
output_name_string= str(sys.argv[2]) #sample identifier
out_dir= str(sys.argv[3])    #sample identifier
outname=out_dir+'/'+output_name_string #+".root" #+'_withTracking.root'#+".root"


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
input_filepath = Path(p.input_files[0])
p.output_files = [outname]

# lower log level so 'info' and above messages can be printed
p.term_log_level=4

# Number of events
p.max_events = 2000
# Where to store DQM plots
p.histogram_file = out_dir+'_hists/'+output_name_string #' #out_dir+'_hists/'+output_name_string #+"_hists.root"
