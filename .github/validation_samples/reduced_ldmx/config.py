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
import LDMX.Hcal.digi as hcal_digi

ecalVeto = ecal_vetos.EcalVetoProcessor()
ecalVeto.num_ecal_layers = 6
ecalVeto.beam_energy = 4000.
ecalVeto.recoil_from_tracking = False

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

from LDMX.Tracking import tracking
from LDMX.Tracking import geo
from LDMX.Tracking import dqm

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
trackgeo.get_instance().setDetector(det)

#smearings
uSmearing = 0.006       #mm #could bump up to 10 micron if we want
vSmearing = 0.000001    #mm #~unused

# Smearing Processor - Recoil
digiRecoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digiRecoil.hit_collection = "RecoilSimHits"
digiRecoil.out_collection = "DigiRecoilSimHits"
digiRecoil.merge_hits = True
digiRecoil.sigma_u = 0.006
digiRecoil.sigma_v = 0.000001

truth_tracking = tracking.LinearTruthTracking("LinearTruthTracking")
truth_tracking.input_hit_collection = "DigiRecoilSimHits"
truth_tracking.input_recHits_collection = "EcalRecHits"
truth_tracking.out_track_collection = "LinearRecoilTruthTracks"

rSeedTracking = tracking.LinearSeedFinder("LinearSeedFinder")
rSeedTracking.input_hit_collection = "DigiRecoilSimHits"
rSeedTracking.input_recHits_collection = "EcalRecHits"
rSeedTracking.out_seed_collection = "LinearRecoilSeedTracks"

rTracking = tracking.LinearTrackFinder("LinearTrackFinder")
rTracking.seed_coll_name = "LinearRecoilSeedTracks"
rTracking.out_trk_collection = "LinearRecoilTracks"

rTracking_dqm = dqm.StraightTracksDQM("LinearRecoilTracksDQM")
rTracking_dqm.track_collection = rTracking.out_trk_collection
rTracking_dqm.truth_collection = truth_tracking.out_track_collection
rTracking_dqm.title = ""
rTracking_dqm.measurement_collection=digiRecoil.out_collection
rTracking_dqm.buildHistograms()

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecalVeto,
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        hcal_veto,
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack, 
        count, TriggerProcessor('trigger', 4000.),
        digiRecoil,
        truth_tracking,
        rSeedTracking,
        rTracking,
        rTracking_dqm])

p.sequence.extend(dqm.all_dqm)