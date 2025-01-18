import os
import sys

from LDMX.Framework import ldmxcfg

thisPassName = 'test'
p=ldmxcfg.Process(thisPassName)

# This need to be set but has no meaning given p.totalEvents
p.maxEvents = 1
# kaon-focused sample takes about twice as long as normal PN,
# so we ask for half as many events such that validation jobs stay
# time-limited by the PN
p.totalEvents = int(os.environ['LDMX_NUM_EVENTS']) // 2
p.run = int(os.environ['LDMX_RUN_NUMBER'])

from LDMX.SimCore import generators as gen
from LDMX.SimCore import bias_operators
from LDMX.SimCore import kaon_physics
from LDMX.SimCore import photonuclear_models as pn
from LDMX.Biasing import ecal
from LDMX.Biasing import filters
from LDMX.Biasing import particle_filter
from LDMX.Biasing import util
from LDMX.Biasing import include as includeBiasing

detector = 'ldmx-det-v14-8gev'
generator=gen.single_8gev_e_upstream_tagger()
bias_factor = 550.
bias_treshold = 5000.

mySim = ecal.photo_nuclear(detector, generator)
mySim.description = f'8 GeV ECal Kaon PN simulation, xsec bias {bias_factor}' 
mySim.biasing_operators = [ bias_operators.PhotoNuclear('ecal',bias_factor,bias_treshold,only_children_of_primary = True) ]

# Configure the sequence in which user actions should be called.
includeBiasing.library()
mySim.actions.clear()
mySim.actions.extend([
        filters.TaggerVetoFilter(thresh=2*3800.),
        # Only consider events where a hard brem occurs
        filters.TargetBremFilter(recoil_max_p = 2*1500.,brem_min_e = 2*2500.),
        # Only consider events where a PN reaction happnes in the ECal
        filters.EcalProcessFilter(),
        # Tag all photo-nuclear tracks to persist them to the event.
        util.TrackProcessFilter.photo_nuclear()
])

# set up "upKaon" parameters which reduces the charged kaon lifetimes by a factor 1/50
# and forces decays to be into one of the leptonic decay modes.
mySim.kaon_parameters = kaon_physics.KaonPhysics.upKaons()

# Alternative pn models
myModel = pn.BertiniAtLeastNProductsModel.kaon() 
# Count all (not stopped) particles as "hard"
myModel.hard_particle_threshold=0.
# Apply the model to any nucleus
myModel.zmin = 0
# Apply the model for photonuclear reactions with > 5000 MeV photons
myModel.emin = 5000.
# PDG ids for K^0_L, K^0_S, K^0, K^+, and K^- respectively
myModel.pdg_ids = [130, 310, 311, 321, -321]
# Require at least 1 hard particle from the list above
myModel.min_products = 1

# Change the default model to the kaon producing model
mySim.photonuclear_model = myModel

# Add the filter at the end of the current list of user actions. 
# Filter for events with a kaon daughter
myFilter = particle_filter.PhotoNuclearProductsFilter.kaon()
mySim.actions.extend([myFilter])

# Load the tracking module
from LDMX.Tracking import tracking
from LDMX.Tracking import geo

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
trackgeo.get_instance().setDetector('ldmx-det-v14-8gev')

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
tracking_tagger.seed_coll_name = seeder_tagger.out_seed_collection
tracking_tagger.out_trk_collection = "TaggerTracks"
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
tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.out_trk_collection = "RecoilTracks"
tracking_recoil.trackID = -1 #1
tracking_recoil.pdgID = -9999 #11
tracking_recoil.measurement_collection = digi_recoil.out_collection
tracking_recoil.min_hits = 6

# Loading calorimeter and TS processors
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions

from LDMX.Ecal import digi as eDigi
from LDMX.Ecal import vetos
from LDMX.Hcal import digi as hDigi
from LDMX.Hcal import hcal
                                                                                                      
from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack

from LDMX.Recon.electronCounter import ElectronCounter

#TS digi + clustering + track chain
tsDigisDown  =TrigScintDigiProducer.pad1()
tsDigisTag   =TrigScintDigiProducer.pad2()
tsDigisUp    =TrigScintDigiProducer.pad3()

tsDigis = [tsDigisDown, tsDigisTag, tsDigisUp]
for d in tsDigis :
    d.randomSeed = 1

tsClustersDown  =TrigScintClusterProducer.pad1()
tsClustersTag  =TrigScintClusterProducer.pad2()
tsClustersUp  =TrigScintClusterProducer.pad3()

tsClustersDown.input_collection = tsDigisDown.output_collection
tsClustersTag.input_collection = tsDigisTag.output_collection
tsClustersUp.input_collection = tsDigisUp.output_collection

#make sure to pick up the right pass 
tsClustersTag.input_pass_name = thisPassName 
tsClustersUp.input_pass_name = tsClustersTag.input_pass_name
tsClustersDown.input_pass_name = tsClustersTag.input_pass_name

trigScintTrack.input_pass_name = thisPassName
trigScintTrack.seeding_collection = tsClustersTag.output_collection

# ECAL part
ecalReco   =eDigi.EcalRecProducer()
ecalDigi = eDigi.EcalDigiProducer()
ecalVeto   =vetos.EcalVetoProcessor()
ecalVeto.recoil_from_tracking = False

# HCAL part
hcalDigi   =hDigi.HcalDigiProducer()
hcalReco   =hDigi.HcalRecProducer()

# electron counter for trigger processor 
eCount = ElectronCounter( 1, "ElectronCounter") # first argument is number of electrons in simulation
eCount.input_pass_name = ''
from LDMX.Recon.simpleTrigger import TriggerProcessor
simpleTrig = TriggerProcessor("simpleTrig",8000.)
simpleTrig.input_pass=thisPassName

# Load DQM 
from LDMX.DQM import dqm
from LDMX.Tracking import dqm as tkdqm

seed_tagger_dqm = tkdqm.TrackingRecoDQM("SeedTaggerDQM")
seed_tagger_dqm.track_collection = seeder_tagger.out_seed_collection
seed_tagger_dqm.truth_collection = "TaggerTruthTracks"
seed_tagger_dqm.title = ""
seed_tagger_dqm.buildHistograms()

tagger_dqm = tkdqm.TrackingRecoDQM("TaggerDQM")
tagger_dqm.track_collection = tracking_tagger.out_trk_collection
tagger_dqm.truth_collection = "TaggerTruthTracks"
tagger_dqm.trackStates = ["target"]
tagger_dqm.title = ""
tagger_dqm.measurement_collection=digi_tagger.out_collection
tagger_dqm.truth_hit_collection="TaggerSimHits"
tagger_dqm.buildHistograms()


seed_recoil_dqm = tkdqm.TrackingRecoDQM("SeedRecoilDQM")
seed_recoil_dqm.track_collection = seeder_recoil.out_seed_collection
seed_recoil_dqm.truth_collection = "RecoilTruthTracks"
seed_recoil_dqm.title = ""
seed_recoil_dqm.buildHistograms()


recoil_dqm = tkdqm.TrackingRecoDQM("RecoilDQM")
recoil_dqm.track_collection = tracking_recoil.out_trk_collection
recoil_dqm.truth_collection = "RecoilTruthTracks"
recoil_dqm.trackStates = ["ecal","target"]
recoil_dqm.title = ""
recoil_dqm.measurement_collection=digi_recoil.out_collection
recoil_dqm.truth_hit_collection="RecoilSimHits"
recoil_dqm.buildHistograms()

# The Tracking modules produce a lot of helpful messages
# but (at the debug level) is too much for commiting the gold log
# into the git working tree on GitHub
p.logger.termLevel = 1
p.logger.custom(truth_tracking, level = 2)
p.logger.custom(digi_tagger, level = 2)
p.logger.custom(digi_recoil, level = 2)
p.logger.custom(seeder_tagger, level = 2)
p.logger.custom(seeder_recoil, level = 2)
p.logger.custom(tracking_tagger, level = 1)
p.logger.custom(tracking_recoil, level = 1)
p.logger.custom(seed_tagger_dqm, level = 2)
p.logger.custom(seed_recoil_dqm, level = 2)
p.logger.custom(tagger_dqm, level = 2)
p.logger.custom(recoil_dqm, level = 2)

# Load HCAL veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()

p.sequence=[ 
        mySim, 
        digi_tagger,
        digi_recoil,
        truth_tracking,
        seeder_tagger,
        seeder_recoil,
        tracking_tagger,
        tracking_recoil,
        ecalDigi, 
        ecalReco, 
        ecalVeto,
        *tsDigis, 
        tsClustersTag,
        tsClustersUp,
        tsClustersDown, 
        trigScintTrack, 
        eCount, 
        simpleTrig, 
        hcalDigi, 
        hcalReco, 
        hcal_veto,
<<<<<<< HEAD
        seed_tagger_dqm,
        tagger_dqm,
        seed_recoil_dqm,
        recoil_dqm,
        dqm.PhotoNuclearDQM(verbose=False)
=======
>>>>>>> 74c64257 (Add HCAL veto in the CI configs)
        seed_tagger_dqm,
        tagger_dqm,
        seed_recoil_dqm,
        recoil_dqm,
        dqm.PhotoNuclearDQM(verbose=False)
        ]

p.sequence.extend(dqm.all_dqm)


p.histogramFile = f'hist.root'
p.outputFiles = [f'events.root']
