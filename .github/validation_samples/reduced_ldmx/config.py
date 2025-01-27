from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxTriesPerEvent = 100

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim

#myGun = gen.single_4gev_e_upstream_tagger()
myGun = gen.multi( "mgpGen" )
myGun.vertex = [ 0., 0., -880] # mm
myGun.momentum = [0.,0.,4000.] # MeV
myGun.nParticles = 1
myGun.pdgID = 11
myGun.enablePoisson = False #True   

mySim = sim.simulator( "mySim" ) # Build simulator object
det = 'ldmx-reduced-v2'
mySim.setDetector(det, True )
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Reduced ECal Electron Gun Test Simulation'

mySim.generators = [ myGun ]
p.sequence = [ mySim ]
p.termLogLevel = 0

import os
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])
p.run = int(os.environ['LDMX_RUN_NUMBER'])

p.histogramFile = f'hist.root'
p.outputFiles = [f'events.root']

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Ecal.EcalWABRecProcessor as ecal_WAB
import LDMX.Hcal.digi as hcal_digi
hcal_digi_reco = hcal_digi.HcalSimpleDigiAndRecProducer()

ecalVeto = ecal_vetos.EcalVetoProcessor()
ecalVeto.num_ecal_layers = 6
ecalVeto.beam_energy = 4000.
ecalVeto.recoil_from_tracking = False
ecalMip = ecal_vetos.EcalMipProcessor()
ecalMip.num_ecal_layers = 6

ecalWAB = ecal_WAB.EcalWABRecProcessor()

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

# Load hcal veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()

from LDMX.DQM import dqm

ecalWAB_dqm = dqm.EcalWABRecResults()

from LDMX.Tracking import tracking
from LDMX.Tracking import reducedTracking
from LDMX.Tracking import geo
from LDMX.Tracking import dqm as trk_dqm

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

rSeedTracking = reducedTracking.LinearSeedFinder("LinearSeedFinder")
rSeedTracking.input_hits_collection = "DigiRecoilSimHits"
rSeedTracking.input_rec_hits_collection = "EcalRecHits"
rSeedTracking.out_seed_collection = "LinearRecoilSeedTracks"
rSeedTracking.layer12_midpoint = layer12_mid
rSeedTracking.layer23_midpoint = layer23_mid
rSeedTracking.layer34_midpoint = layer34_mid
rSeedTracking.recoil_uncertainty = [0.006, 0.085]
rSeedTracking.ecal_distance_threshold = 15.0

rTracking = reducedTracking.LinearTrackFinder("LinearTrackFinder")
rTracking.seed_collection = "LinearRecoilSeedTracks"
rTracking.out_trk_collection = "LinearRecoilTracks"

rTracking_dqm = trk_dqm.StraightTracksDQM("LinearRecoilTracksDQM")
rTracking_dqm.track_collection = rTracking.out_trk_collection
rTracking_dqm.truth_collection = truth_tracking.out_track_collection
rTracking_dqm.title = ""
rTracking_dqm.measurement_collection=digi_recoil_reduced.out_collection
rTracking_dqm.buildHistograms()

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecalVeto,
        ecalMip,
        hcal_digi_reco,
        hcal_veto,
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack, 
        count, TriggerProcessor('trigger', 4000.),
        digi_recoil_reduced,
        truth_tracking,
        rSeedTracking,
        rTracking,
        rTracking_dqm,
	ecalWAB,
	ecalWAB_dqm])

p.sequence.extend(dqm.all_dqm)
