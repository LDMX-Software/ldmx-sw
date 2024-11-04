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
det = 'ldmx-reduced-v1'
mySim.setDetector(det, True )
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Reduced ECal Electron Gun Test Simulation'

mySim.generators = [ myGun ]
p.sequence = [ mySim ]
p.termLogLevel = 0


p.maxEvents = 5000
p.run = 200

p.histogramFile = f'hist_linearTruth_dqmTEST.root'
p.outputFiles = [f'events_5000_linearUsingTruth_reducedV1.root']

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi

ecalVeto = ecal_vetos.EcalVetoProcessor()
ecalVeto.num_ecal_layers = 6

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

from LDMX.DQM import dqm

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecalVeto,
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack, 
        count, TriggerProcessor('trigger', 4000.),
#        ] + dqm.all_dqm)
        ])
        
from LDMX.Tracking import tracking
from LDMX.Tracking import geo

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
trackgeo.get_instance().setDetector('ldmx-reduced-v1')

# Truth seeder
truth_tracking           = tracking.TruthSeedProcessor()
truth_tracking.debug             = True
truth_tracking.trk_coll_name     = "RecoilTruthSeeds"
#truth_tracking.pdgIDs            = [11]
truth_tracking.scoring_hits      = "TargetScoringPlaneHits"
truth_tracking.z_min             = 0.
truth_tracking.track_id          = -1
truth_tracking.p_cut             = 0.05 # In GeV
truth_tracking.pz_cut            = 0.03
truth_tracking.p_cutEcal         = 0. # In GeV
truth_tracking.n_min_hits_recoil = 3

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

#Recoil tracking
#CKF Options
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.dumpobj = False
tracking_recoil.debug = False
tracking_recoil.propagator_step_size = 1000.  #mm
tracking_recoil.bfield = 0.0  #in T #From looking at the BField map #not used if below is False
tracking_recoil.const_b_field = True

#Target location for the CKF extrapolation
tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.out_trk_collection = "RecoilTracks"

#smear the hits used for finding/fitting
tracking_recoil.trackID = -1 #1
tracking_recoil.pdgID = -9999 #11
tracking_recoil.measurement_collection = digiRecoil.out_collection
tracking_recoil.min_hits = 2

from LDMX.Tracking import dqm

recoil_dqm = dqm.TrackingRecoDQM("RecoilDQM")
recoil_dqm.track_collection = tracking_recoil.out_trk_collection
recoil_dqm.truth_collection = "RecoilTruthTracks"
recoil_dqm.trackStates = ["ecal","target"]
recoil_dqm.title = ""
recoil_dqm.measurement_collection=digiRecoil.out_collection
recoil_dqm.truth_hit_collection="RecoilSimHits"
recoil_dqm.buildHistograms()

p.sequence.extend([digiRecoil, truth_tracking, tracking_recoil, recoil_dqm])
