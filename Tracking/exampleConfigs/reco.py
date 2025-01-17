# Example of jobOption to run tracking on input simulated files in ldmx-sw
# For detailed description of the various configurations, check the .py module files inside
# Tracking/python

import os,math
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

p = ldmxcfg.Process("TrackerReco")

# Load the tracking module
from LDMX.Tracking import tracking

# This has to stay after defining the "TrackerReco" Process in order to load the geometry
# From the conditions
from LDMX.Tracking import geo

n_evts=1000  #number of events to gen/reco

#   set up a simple particle gun for this example  #
#   just 8gev electrons started upstream of tagger and first ts #
partGunString='single_8gev_e_upstream_tagger'
detector = 'ldmx-det-v14-8gev-no-cals'
####  set up beam simulation
sim = simulator.simulator('inclusive_single_8gev')
sim.setDetector(detector,include_scoring_planes = True)  
sim.description = 'A single 8gev electron shot from upstream of the 8gev tagger.'
sim.beamSpotSmear = [20., 80., 0]
particle_gun = generators.single_8gev_e_upstream_tagger()
sim.generators.append(particle_gun)
####  end beam simulation

# Truth seeder 
# Runs truth tracking producing tracks from target scoring plane hits for Recoil
# and generated electros for Tagger.
# Truth tracks can be used for assessing tracking performance or using as seeds
truth_tracking           = tracking.TruthSeedProcessor("TruthSeeds")
truth_tracking.debug             = False
#truth_tracking.trk_coll_name     = "RecoilTruthSeeds"
#truth_tracking.pdgIDs            = [11]
#truth_tracking.scoring_hits      = "TargetScoringPlaneHits"
#truth_tracking.z_min             = 0.
#truth_tracking.track_id          = -1
#truth_tracking.p_cut             = 0.05 # In MeV


# These smearing quantities are default. We expect around 6um hit resolution in bending plane
# v-smearing is actually not used as 1D measurements are used for tracking. These smearing parameters
# are fed to the digitization producer.
uSmearing = 0.006       #mm
vSmearing = 0.000001    #mm


# Smearing Processor - Tagger
# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to reduce combinatorics
digi_tagger = tracking.DigitizationProcessor("DigitizationProcessor")
digi_tagger.hit_collection = "TaggerSimHits"
digi_tagger.out_collection = "DigiTaggerSimHits"
digi_tagger.merge_hits = True
digi_tagger.sigma_u = uSmearing
digi_tagger.sigma_v = vSmearing

# Smearing Processor - Recoil
digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digi_recoil.hit_collection = "RecoilSimHits"
digi_recoil.out_collection = "DigiRecoilSimHits"
digi_recoil.merge_hits = True
digi_recoil.sigma_u = uSmearing
digi_recoil.sigma_v = vSmearing


# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at the track
# parameters and the impact parameters at the target or generation point. For the tagger one should look
# for compatibility with the beam orbit / beam spot

seeder_tagger = tracking.SeedFinderProcessor("SeedTagger")
seeder_tagger.input_hits_collection =  digi_tagger.out_collection
seeder_tagger.out_seed_collection = "TaggerRecoSeeds"
seeder_tagger.pmin  = 0.1
seeder_tagger.pmax  = 10.0
seeder_tagger.d0min = -45.
seeder_tagger.d0max = 45.
seeder_tagger.z0max = 60.

#Seed finder processor - Recoil
seeder_recoil = tracking.SeedFinderProcessor("SeedRecoil")
seeder_recoil.perigee_location = [0.,0.,0.]
seeder_recoil.input_hits_collection =  digi_recoil.out_collection
seeder_recoil.out_seed_collection = "RecoilRecoSeeds"
seeder_recoil.bfield = 1.5
seeder_recoil.pmin  = 0.1
seeder_recoil.pmax  = 10.0
seeder_recoil.d0min = -40.0
seeder_recoil.d0max = 40.0
seeder_recoil.z0max = 50.


# Producer for running the CKF track finding starting from the found seeds.
tracking_tagger  = tracking.CKFProcessor("Tagger_TrackFinder")
tracking_tagger.dumpobj = False
tracking_tagger.debug = True
tracking_tagger.propagator_step_size = 1000.  #mm
tracking_tagger.bfield = -1.5  #in T #From looking at the BField map
tracking_tagger.const_b_field = False

#Target location for the CKF extrapolation
tracking_tagger.seed_coll_name = seeder_tagger.out_seed_collection
tracking_tagger.out_trk_collection = "TaggerTracks"

#smear the hits used for finding/fitting
tracking_tagger.trackID = -1 #1
tracking_tagger.pdgID = -9999 #11
tracking_tagger.measurement_collection = digi_tagger.out_collection
tracking_tagger.min_hits = 6


#CKF Options
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.dumpobj = False
tracking_recoil.debug = True
tracking_recoil.propagator_step_size = 1000.  #mm
tracking_recoil.bfield = -1.5  #in T #From looking at the BField map
tracking_recoil.const_b_field = False
tracking_recoil.taggerTracking=False
#Target location for the CKF extrapolation
#tracking_recoil.seed_coll_name = seeder_recoil.out_seed_collection
tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.out_trk_collection = "RecoilTracks"

#smear the hits used for finding/fitting
tracking_recoil.trackID = -1 #1
tracking_recoil.pdgID = -9999 #11
tracking_recoil.measurement_collection = digi_recoil.out_collection
tracking_recoil.min_hits = 6


from LDMX.Tracking import dqm

seed_tagger_dqm = dqm.TrackingRecoDQM("SeedTaggerDQM")
seed_tagger_dqm.track_collection = seeder_tagger.out_seed_collection
seed_tagger_dqm.truth_collection = "TaggerTruthTracks"
seed_tagger_dqm.title = ""
seed_tagger_dqm.buildHistograms()

tagger_dqm = dqm.TrackingRecoDQM("TaggerDQM")
tagger_dqm.track_collection = tracking_tagger.out_trk_collection
tagger_dqm.truth_collection = "TaggerTruthTracks"
tagger_dqm.trackStates = ["target"]
tagger_dqm.title = ""
tagger_dqm.measurement_collection=digi_tagger.out_collection
tagger_dqm.truth_hit_collection="TaggerSimHits"
tagger_dqm.buildHistograms()


seed_recoil_dqm = dqm.TrackingRecoDQM("SeedRecoilDQM")
seed_recoil_dqm.track_collection = seeder_recoil.out_seed_collection
seed_recoil_dqm.truth_collection = "RecoilTruthTracks"
seed_recoil_dqm.title = ""
seed_recoil_dqm.buildHistograms()

recoil_dqm = dqm.TrackingRecoDQM("RecoilDQM")
recoil_dqm.track_collection = tracking_recoil.out_trk_collection
recoil_dqm.truth_collection = "RecoilTruthTracks"
recoil_dqm.trackStates = ["ecal","target"]
recoil_dqm.title = ""
recoil_dqm.measurement_collection=digi_recoil.out_collection
recoil_dqm.truth_hit_collection="RecoilSimHits"
recoil_dqm.buildHistograms()

# This sequence runs the digitization in the tagger and recoil
# Then the truth tracking to have TruthTracks in the final state
# the nominal seeding is ran on the tagger and recoil
# Track finding is then raun in the tagger and in the recoil
# Finally two dqm examples are run in the recoil tracks and using the seed tracks

p.sequence   = [sim,truth_tracking,digi_tagger, digi_recoil,
                seeder_tagger, seeder_recoil,
                tracking_tagger, tracking_recoil,
                recoil_dqm, seed_recoil_dqm,
                tagger_dqm, seed_tagger_dqm]

# Output name
#   just append '_withTracking' to the name of the input file
from pathlib import Path
p.outputFiles = ['test_8gev_electrons_withTracking.root']

# lower log level so 'info' and above messages can be printed
p.termLogLevel=1

# Number of events
p.maxEvents = n_evts

# Where to store DQM plots
p.histogramFile = "test_dqmMonitoringFile.root"

