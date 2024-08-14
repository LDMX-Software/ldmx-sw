from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxTriesPerEvent = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
det = 'ldmx-det-v14-8gev'
mySim = ecal.photo_nuclear(det,gen.single_8gev_e_upstream_tagger())
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'ECal PN Test Simulation'

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])
p.run = int(os.environ['LDMX_RUN_NUMBER'])

p.histogramFile = f'hist.root'
p.outputFiles = [f'events.root']

# The Tracking modules produce a lot of helpful messages
# but (at the debug level) is too much for commiting the gold log
# into the git working tree on GitHub
p.termLogLevel = 1

# Load the tracking module
from LDMX.Tracking import tracking
from LDMX.Tracking import geo

# Truth seeder
# Runs truth tracking producing tracks from target scoring plane hits for Recoil
# and generated electros for Tagger.
# Truth tracks can be used for assessing tracking performance or using as seeds
truth_tracking           = tracking.TruthSeedProcessor()
truth_tracking.debug             = True
truth_tracking.trk_coll_name     = "RecoilTruthSeeds"
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

seederTagger = tracking.SeedFinderProcessor()
seederTagger.input_hits_collection =  digiTagger.out_collection
seederTagger.out_seed_collection = "TaggerRecoSeeds"
seederTagger.pmin  = 2.
seederTagger.pmax  = 12.
seederTagger.d0min = -60.
seederTagger.d0max = 0.

#Seed finder processor - Recoil
seederRecoil = tracking.SeedFinderProcessor("SeedRecoil")
seederRecoil.perigee_location = [0.,0.,0.]
seederRecoil.input_hits_collection =  digiRecoil.out_collection
seederRecoil.out_seed_collection = "RecoilRecoSeeds"
seederRecoil.bfield = 1.5
seederRecoil.pmin  = 0.1
seederRecoil.pmax  = 4.
seederRecoil.d0min = -0.5
seederRecoil.d0max = 0.5
seederRecoil.z0max = 10.

# Producer for running the CKF track finding starting from the found seeds.
tracking_tagger  = tracking.CKFProcessor("Tagger_TrackFinder")
tracking_tagger.dumpobj = False
tracking_tagger.debug = True
tracking_tagger.propagator_step_size = 1000.  #mm
tracking_tagger.bfield = -1.5  #in T #From looking at the BField map
tracking_tagger.const_b_field = False

#Target location for the CKF extrapolation
tracking_tagger.seed_coll_name = seederTagger.out_seed_collection
tracking_tagger.out_trk_collection = "TaggerTracks"

#smear the hits used for finding/fitting
tracking_tagger.trackID = -1 #1
tracking_tagger.pdgID = -9999 #11
tracking_tagger.measurement_collection = digiTagger.out_collection
tracking_tagger.min_hits = 5


#CKF Options
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.dumpobj = False
tracking_recoil.debug = True
tracking_recoil.propagator_step_size = 1000.  #mm
tracking_recoil.bfield = -1.5  #in T #From looking at the BField map
tracking_recoil.const_b_field = False

#Target location for the CKF extrapolation
#tracking_recoil.seed_coll_name = seederRecoil.out_seed_collection
tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.out_trk_collection = "RecoilTracks"

#smear the hits used for finding/fitting
tracking_recoil.trackID = -1 #1
tracking_recoil.pdgID = -9999 #11
tracking_recoil.measurement_collection = digiRecoil.out_collection
tracking_recoil.min_hits = 5

# Load the ECAL modules
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos

# Load the HCAL modules
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Hcal.digi as hcal_digi

# Load the TS modules
from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack
ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]
for d in ts_digis :
    d.randomSeed = 1

from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

# Load the DQM modules
from LDMX.DQM import dqm
from LDMX.Tracking import dqm as tkdqm

# currently this does not fill anything
#seed_recoil_dqm = tkdqm.TrackingRecoDQM("SeedRecoilTrackerDQM")
#seed_recoil_dqm.buildHistograms()
#seed_recoil_dqm.track_collection = seederRecoil.out_seed_collection
#seed_recoil_dqm.truth_collection = "RecoilTruthTracks"
#seed_recoil_dqm.title = ""

recoil_dqm = tkdqm.TrackingRecoDQM("RecoilTrackerDQM")
recoil_dqm.buildHistograms()
recoil_dqm.track_collection = tracking_recoil.out_trk_collection
recoil_dqm.truth_collection = "RecoilTruthTracks"
recoil_dqm.title = ""

p.sequence.extend([
        digiTagger,
        digiRecoil,
        truth_tracking,
        seederTagger,
        seederRecoil,
        tracking_tagger,
        tracking_recoil,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecal_vetos.EcalVetoProcessor(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack, 
        count, TriggerProcessor('trigger', 8000.),
        dqm.PhotoNuclearDQM(verbose=True),
#        seed_recoil_dqm,
        recoil_dqm,
        ] + dqm.all_dqm)
