import argparse

parser = argparse.ArgumentParser()
parser.add_argument(
    '--max-energy',
    help='maximum energy [GeV] to sample from',
    default=4.0,
    type=float,
)
parser.add_argument(
    '--min-energy',
    help='minimum energy [GeV] to sample from',
    default=0.0,
    type=float,
)
parser.add_argument(
    '--angle',
    help='maximum polar angle [degrees] to sample from',
    default=60.0,
    type=float,
)
parser.add_argument(
    '--n-events',
    help='number of events to simulate',
    default=10,
    type=int,
)
args = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('uniele')
p.max_events = args.n_events
p.run = 1

filename = (
    f'uniform_electrons_maxE_{args.max_energy}'
    f'_minE_{args.min_energy}'
    f'_maxPolar_{args.angle}'
    f'_N_{args.n_events}'
    f'_run_{p.run:04d}.root'
)
p.output_files = [ 'events_'+filename ]
p.histogram_file = 'hists_'+filename

import LDMX.Hcal.hcal_geometry
import LDMX.Ecal.ecal_geometry

from LDMX.SimCore import simulator
from LDMX.SimCore import generators

sim = simulator.Simulator("uniform-electrons")
sim.set_detector('ldmx-det-v14-8gev', include_scoring_planes_minimal = True)
sim.description = "Electrons with uniformly sampled energy and angle shot from target"
sim.beamSpotSmear = [20., 80., 0.]
# GPS generator
sim.generators = [
    generators.Gps('uniform-electrons', [
        # electrons
        '/gps/particle e-',
        # position distribution: all from the same point, simulator smears beam spot
        '/gps/pos/type Point', # beamSpotSmear will smear for us
        '/gps/pos/centre 0 0 0 mm', # shoot from center of target
        # angular distribution, isotropic with maximum polar angle
        # relative to z-axis
        '/gps/direction 0 0 1',
        # the default direction is negative z (like cosmics coming down from the
        # sky), so we need to rotate the frame of the angular distribution to be
        # pointed along
        # positive z
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
truth_tracking.pdg_ids            = [11]
truth_tracking.scoring_hits      = "TargetScoringPlaneHits"
truth_tracking.z_min             = 0.
truth_tracking.track_id          = -1
truth_tracking.p_cut             = 0.05 # In MeV
truth_tracking.pz_cut            = 0.03
truth_tracking.p_cut_ecal         = 0. # In MeV

# These smearing quantities are default. We expect around 6um hit resolution in bending
# plane
# v-smearing is actually not used as 1D measurements are used for tracking. These
# smearing parameters
# are fed to the digitization producer.
u_smearing = 0.006       #mm
v_smearing = 0.000001    #mm

# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to
# reduce combinatorics
# Smearing Processor - Recoil
digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digi_recoil.hit_collection = "RecoilSimHits"
digi_recoil.out_collection = "DigiRecoilSimHits"
digi_recoil.merge_hits = True
digi_recoil.sigma_u = u_smearing
digi_recoil.sigma_v = v_smearing


# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting
# them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at
# the track
# parameters and the impact parameters at the target or generation point. For the tagger
# one should look
# for compatibility with the beam orbit / beam spot
#Seed finder processor - Recoil
seeder_recoil = tracking.SeedFinderProcessor("SeedRecoil")
seeder_recoil.perigee_location = [0.,0.,0.]
seeder_recoil.input_hits_collection =  digi_recoil.out_collection
seeder_recoil.out_seed_collection = "RecoilRecoSeeds"
seeder_recoil.bfield = 1.5
seeder_recoil.pmin  = 0.1
seeder_recoil.pmax  = 4.
seeder_recoil.d0min = -0.5
seeder_recoil.d0max = 0.5
seeder_recoil.z0max = 10.


# Producer for running the CKF track finding starting from the found seeds.
#CKF Options
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.dumpobj = False
tracking_recoil.debug = True
tracking_recoil.propagator_step_size = 1000.  #mm
tracking_recoil.bfield = -1.5  #in T #From looking at the BField map
tracking_recoil.const_b_field = False

#Target location for the CKF extrapolation
#tracking_recoil.seed_coll_name = seeder_recoil.out_seed_collection
tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.out_trk_collection = "RecoilTracks"

#smear the hits used for finding/fitting
tracking_recoil.trackID = -1 #1
tracking_recoil.pdg_id = -9999 #11
tracking_recoil.measurement_collection = digi_recoil.out_collection
tracking_recoil.min_hits = 5

from LDMX.Tracking import dqm
digi_dqm = dqm.TrackerDigiDQM()
tracking_dqm = dqm.TrackingRecoDQM()

seed_recoil_dqm = dqm.TrackingRecoDQM("SeedRecoilDQM")
seed_recoil_dqm.track_collection = seeder_recoil.out_seed_collection
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
    digi_recoil,
    truth_tracking,
    seeder_recoil,
    tracking_recoil,
    recoil_dqm, seed_recoil_dqm
]
