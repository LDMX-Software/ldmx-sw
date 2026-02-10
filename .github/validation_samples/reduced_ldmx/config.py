from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('test')

p.max_tries_per_event = 100

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim


#my_gun = gen.single_4gev_e_upstream_tagger()
my_gun = gen.multi( "mgpGen" )
my_gun.vertex = [ 0., 0., -880] # mm
my_gun.momentum = [0.,0.,4000.] # MeV
my_gun.n_particles = 1
my_gun.pdg_id = 11
my_gun.enable_poisson = False #True

my_sim = sim.simulator( "my_sim" ) # Build simulator object
det = 'ldmx-reduced-v3'
my_sim.setDetector(det, include_scoring_planes_minimal = True )
my_sim.beamSpotSmear = [20.,80.,0.]
my_sim.description = 'Reduced ECal Electron Gun Test Simulation'

my_sim.generators = [ my_gun ]
p.sequence = [ my_sim ]
p.term_log_level = 0

import os


p.max_events = int(os.environ['LDMX_NUM_EVENTS'])
p.run = int(os.environ['LDMX_RUN_NUMBER'])

p.histogram_file = 'hist.root'
p.output_files = ['events.root']

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.EcalWABRecProcessor as ecal_WAB
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi_and_reco
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions


hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()

ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_veto.num_ecal_layers = 4
ecal_veto.beam_energy = 4000.
ecal_veto.recoil_from_tracking = False

ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_mip.num_ecal_layers = 4

ecal_wab = ecal_WAB.EcalWABRecProcessor()

from LDMX.TrigScint.trigScint import (
    TrigScintClusterProducer,
    TrigScintDigiProducer,
    trig_scint_track,
)


ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]
for d in ts_digis :
    d.randomSeed = 1

from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

# Load hcal veto
import LDMX.Hcal.hcal as hcal


hcal_veto = hcal.HcalVetoProcessor()

from LDMX.DQM import dqm


ecalWAB_dqm = dqm.EcalWABRecResults()

from LDMX.Tracking import dqm as trk_dqm
from LDMX.Tracking import geo, reducedTracking, tracking
from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo


trackgeo.get_instance().setDetector(det)

# Smearing Processor - Recoil
digi_recoil_reduced = tracking.DigitizationProcessor("DigitizationProcessorRecoilReduced")
digi_recoil_reduced.hit_collection = "RecoilSimHits"
digi_recoil_reduced.out_collection = "DigiRecoilSimHits"
digi_recoil_reduced.merge_hits = True
digi_recoil_reduced.sigma_u = 0.006
digi_recoil_reduced.sigma_v = 0.000001

#Midpoints between recoil layers. Used to classify hits by layer
#Need to be manually updated for geometry changes that move the Recoil positions
layer12_mid = (9.5+15.5)/2.
layer23_mid = (15.5+24.5)/2.
layer34_mid = (24.5+30.5)/2.

truth_tracking = reducedTracking.LinearTruthTracking("LinearTruthTracking")
truth_tracking.input_hits_collection = "RecoilSimHits"
truth_tracking.input_rec_hits_collection = "EcalRecHits"
truth_tracking.out_track_collection = "LinearRecoilTruthTracks"
truth_tracking.layer12_midpoint = layer12_mid
truth_tracking.layer23_midpoint = layer23_mid
truth_tracking.layer34_midpoint = layer34_mid

r_seed_tracking = reducedTracking.LinearSeedFinder("LinearSeedFinder")
r_seed_tracking.input_hits_collection = "DigiRecoilSimHits"
r_seed_tracking.input_rec_hits_collection = "EcalRecHits"
r_seed_tracking.out_seed_collection = "LinearRecoilSeedTracks"
r_seed_tracking.layer12_midpoint = layer12_mid
r_seed_tracking.layer23_midpoint = layer23_mid
r_seed_tracking.layer34_midpoint = layer34_mid
r_seed_tracking.recoil_uncertainty = [0.006, 0.085]
r_seed_tracking.ecal_distance_threshold = 15.0

r_tracking = reducedTracking.LinearTrackFinder("LinearTrackFinder")
r_tracking.seed_collection = "LinearRecoilSeedTracks"
r_tracking.out_trk_collection = "LinearRecoilTracks"

rTracking_dqm = trk_dqm.StraightTracksDQM("LinearRecoilTracksDQM")
rTracking_dqm.track_collection = r_tracking.out_trk_collection
rTracking_dqm.truth_collection = truth_tracking.out_track_collection
rTracking_dqm.title = ""
rTracking_dqm.measurement_collection=digi_recoil_reduced.out_collection
rTracking_dqm.buildHistograms()

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_veto,
        ecal_mip,
        hcal_digi,
        hcal_reco,
        hcal_veto,
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trig_scint_track,
        count, TriggerProcessor('trigger', 4000.),
        digi_recoil_reduced,
        truth_tracking,
        r_seed_tracking,
        r_tracking,
        rTracking_dqm,
	ecal_wab,
	ecalWAB_dqm])

reduced_ecal_dqm = [
        dqm.EcalDigiVerify(),
        dqm.EcalShowerFeatures(),
        dqm.EcalMipTrackingFeatures(),
        dqm.EcalVetoResults()
        ]

reduced_dqm = [dqm.sample_validation_dqm + reduced_ecal_dqm +  dqm.hcal_dqm +  dqm.trigScint_dqm +  dqm.trigger_dqm]
p.sequence.extend(*reduced_dqm)
