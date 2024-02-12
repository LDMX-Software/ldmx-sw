import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--max-energy',help='maximum energy [GeV] to sample from', default=4.0, type=float)
parser.add_argument('--min-energy',help='minimum energy [GeV] to sample from', default=0.0, type=float)
parser.add_argument('--angle',help='maximum polar angle [degrees] to sample from', default=60.0, type=float)
parser.add_argument('--n-events',help='number of events to simulate',default=10,type=int)
args = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('uniele')
p.maxEvents = args.n_events
p.run = 1

filename = f'uniform_electrons_maxE_{args.max_energy}_minE_{args.min_energy}_maxPolar_{args.angle}_N_{args.n_events}_run_{p.run:04d}.root'
p.outputFiles = [ 'events_'+filename ]
p.histogramFile = 'hists_'+filename

import LDMX.Hcal.HcalGeometry
import LDMX.Ecal.EcalGeometry

from LDMX.SimCore import simulator
from LDMX.SimCore import generators

sim = simulator.simulator("uniform-electrons")
sim.setDetector('ldmx-det-v14-8gev', True)
sim.description = "Electrons with uniformly sampled energy and angle shot from target"
sim.beamSpotSmear = [20., 80., 0.]
# GPS generator
sim.generators = [
    generators.gps('uniform-electrons', [
        # electrons
        '/gps/particle e-',
        # position distribution: all from the same point, simulator smears beam spot
        '/gps/pos/type Point', # beamSpotSmear will smear for us
        '/gps/pos/centre 0 0 0 mm', # shoot from center of target
        # angular distribution, isotropic with maximum polar angle relative to z-axis
        '/gps/direction 0 0 1',
        # the default direction is negative z (like cosmics coming down from the sky)
        # so we need to rotate the frame of the angular distribution to be pointed along positive z
        '/gps/ang/rot1 1 0 0',
        '/gps/ang/rot2 0 -1 0',
        '/gps/ang/type cos', # isotropic angular distribution
        '/gps/ang/mintheta 0 deg', # minimum polar angle
        f'/gps/ang/maxtheta {args.angle} deg', # maximum polar angle
        '/gps/ang/minphi 0 deg', # minimum azimuthal angle
        '/gps/ang/maxphi 360 deg', # maximum azimuthal angle
        # energy distribution, uniform between the two configured limits
        '/gps/ene/type Lin', # linear distribution (will set slope to zero)
        f'/gps/ene/min {args.min_energy} GeV',
        f'/gps/ene/max {args.max_energy} GeV',
        '/gps/ene/gradient 0', # make linear distribution flat
        '/gps/ene/intercept 1',
        # one particle per event
        '/gps/number 1',
    ])
]

from LDMX.Tracking import tracking

import LDMX.Tracking.geo

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

# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to reduce combinatorics
# Smearing Processor - Recoil
digiRecoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digiRecoil.hit_collection = "RecoilSimHits"
digiRecoil.out_collection = "DigiRecoilSimHits"
digiRecoil.merge_hits = True
digiRecoil.sigma_u = uSmearing
digiRecoil.sigma_v = vSmearing


# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at the track
# parameters and the impact parameters at the target or generation point. For the tagger one should look
# for compatibility with the beam orbit / beam spot
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

from LDMX.Tracking import dqm
digi_dqm = dqm.TrackerDigiDQM()
tracking_dqm = dqm.TrackingRecoDQM()

seed_recoil_dqm = dqm.TrackingRecoDQM("SeedRecoilDQM")
seed_recoil_dqm.track_collection = seederRecoil.out_seed_collection
seed_recoil_dqm.truth_collection = "RecoilTruthTracks"
seed_recoil_dqm.title = ""

recoil_dqm = dqm.TrackingRecoDQM("RecoilDQM")
recoil_dqm.track_collection = tracking_recoil.out_trk_collection
recoil_dqm.truth_collection = "RecoilTruthTracks"
recoil_dqm.title = ""

# This sequence runs the digitization in the tagger and recoil
# Then the truth tracking to have TruthTracks in the final state
# the nominal seeding is ran on the tagger and recoil
# Track finding is then raun in the tagger and in the recoil
# Finally two dqm examples are run in the recoil tracks and using the seed tracks

p.sequence = [
    sim,
    digiRecoil,
    truth_tracking,
    seederRecoil,
    tracking_recoil,
    recoil_dqm, seed_recoil_dqm
]
