from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('rLDMX_vEXP')

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
det = 'ldmx-reduced-experiment'
mySim.setDetector(det, True )
mySim.beamSpotSmear = [20.,80.,0.]
#mySim.beamSpotSmear = [0.,0.,0.]
mySim.description = 'Electron Gun Test: Reduced LDMX Tracking Algorithm'

mySim.generators = [ myGun ]
p.sequence = [ mySim ]
p.termLogLevel = 0


p.maxEvents = 5000
p.run = 200

p.histogramFile = f'hist_reducedAlgoV1.root'
p.outputFiles = [f'events_5000_rLDMX_vEXP_dSensor27half.root']
#p.outputFiles = [f'test.root']


import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos

import LDMX.Hcal.HcalGeometry

ecalVeto = ecal_vetos.EcalVetoProcessor()
ecalVeto.num_ecal_layers = 6

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
    
trigScintTrack.delta_max = 0.75
    
p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecalVeto,
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack,])
        
from LDMX.Tracking import tracking
from LDMX.Tracking import geo

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
trackgeo.get_instance().setDetector('ldmx-reduced-experiment')

#smearings
uSmearing = 0.006       #mm #could bump up to 10 micron if we want
vSmearing = 0.000001    #mm #~unused

# Smearing Processor - Recoil
digiRecoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digiRecoil.hit_collection = "RecoilSimHits"
digiRecoil.out_collection = "DigiRecoilSimHits"
digiRecoil.merge_hits = True
digiRecoil.sigma_u = uSmearing
digiRecoil.sigma_v = vSmearing

layer12_mid = (9.5+15.5)/2.
layer23_mid = (15.5+27.5)/2.
layer34_mid = (27.5+33.5)/2.

truth_tracking = tracking.LinearTruthTracking("LinearTruthTracking")
truth_tracking.input_hit_collection = "DigiRecoilSimHits"
truth_tracking.input_recHits_collection = "EcalRecHits"
truth_tracking.out_track_collection = "LinearRecoilTruthTracks"
truth_tracking.layer12_midpoint = layer12_mid
truth_tracking.layer23_midpoint = layer23_mid
truth_tracking.layer34_midpoint = layer34_mid

rSeedTracking = tracking.LinearSeedFinder("LinearSeedFinder")
rSeedTracking.input_hit_collection = "DigiRecoilSimHits"
rSeedTracking.input_recHits_collection = "EcalRecHits"
rSeedTracking.out_seed_collection = "LinearRecoilSeedTracks"
rSeedTracking.layer12_midpoint = layer12_mid
rSeedTracking.layer23_midpoint = layer23_mid
rSeedTracking.layer34_midpoint = layer34_mid
#rSeedTracking.ecal_distance_threshold = 100.0

rTracking = tracking.LinearTrackFinder("LinearTrackFinder")
rTracking.seed_coll_name = "LinearRecoilSeedTracks"
rTracking.out_trk_collection = "LinearRecoilTracks"

p.sequence.extend([digiRecoil, truth_tracking, rSeedTracking, rTracking])
