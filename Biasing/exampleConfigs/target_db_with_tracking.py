import argparse
import os

parser = argparse.ArgumentParser(
    description = "An ldmx-sw config file for simulating dark brem ineractions \
                   in the LDMX tungsten target. \
                   Includes full tracking for both tagger and recoil trackers. \
                   Includes optional correction to forward-only energy scaling \
                   to approximate expected lepton backscattering.",
    epilog = "This config will work with default db-lib-gen output libraries \
              if no arguments are given and no changes are made. \
              If db-lib-gen parameters are customized, \
              the user should change the 'db_lib' variable in this script accordingly. \
              Supplying arguments is useful to correct forward only scaling \
              and/or produce many root files concurrently."
    )
parser.add_argument('--correct-forward', action='store_true', default=False,
                    help='apply backscatter estimation to forward only scaling')
parser.add_argument('-r', '--run', type=int, default=3000,
                    help='run number of the input db library (Default: 3000)')
parser.add_argument('-n', '--nevents', type=int, default=1000,
                    help='maximum number of events to simulate (Default: 1000)')
parser.add_argument('-m', '--apmass', type=float, default=100.,
                    help='mass of A prime in MeV (Default: 100)')
parser.add_argument('-l', '--aprime-lhe', type=int, default=1023,
                    help='A prime lhe id (Default: 1023)')
parser.add_argument('-d', '--data-dir', type=str, default='',
                    help='directory with input db libraries')
parser.add_argument('-o', '--out-dir', type=str, default='',
                    help='directory to save output root file(s)')
args = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('tdb')
p.run = args.run
import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.Biasing import target
from LDMX.SimCore import generators

detector = 'ldmx-det-v15-8gev' # name of geometry to use

# Rrobably easiest to change 'db_lib' by hand instead of passing numerous arguments.
# This specifies the input db library.
# The default db-lib-gen output will work if no changes are made.
# Use the following syntax and insert whichever parameters you supplied to db-lib-gen
# {lepton}_{target}_MaxE_{max_energy}_MinE_{min_energy}_RelEStep_{rel_step}
# _UndecayedAP_mA_{apmass}_run_{run}
apmass_gev = args.apmass / 1000.
db_lib = f'electron_tungsten_MaxE_8.0_MinE_4.0_RelEStep_0.1 \
           _UndecayedAP_mA_{apmass_gev}_run_{args.run}'
target_ap_sim = target.dark_brem(
                  args.apmass, #MeV - mass of A' (default 100)
                  db_lib if not args.data_dir else os.path.join(args.data_dir, db_lib),
                  detector,
                  generators.single_8gev_e_upstream_tagger(), # electron gun
                  correct_forward = args.correct_forward, # apply correction or not
                  aprime_lhe_id = args.aprime_lhe # may need to change to e.g., 622
                )

# Add target dark brem to the process sequence
p.sequence = [
    target_ap_sim
    ]

from LDMX.Tracking import tracking
from LDMX.Tracking import geo
# Load the full tracking sequence
from LDMX.Tracking.full_tracking_sequence import full_tracking_sequence

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity
# resolution, GSF, DQM
trk = full_tracking_sequence(detector=detector)
p.sequence.extend(trk.sequence)
p.sequence.extend(trk.dqm_sequence)

# Add DQM for dark brem interaction
# Uses sim particles to analyze kinematics of dark brem interaction
from LDMX.DQM import dqm
p.sequence.extend([dqm.DarkBremInteraction()])

p.max_events = args.nevents
p.max_tries_per_event = 1000

if args.out_dir:
    out = os.path.join(args.out_dir, f'tdb_{args.run}.root')
    hist_out = os.path.join(args.out_dir, f'dqmMonitoringFile_{args.run}.root')
    p.output_files = [ out ]
    p.histogram_file = hist_out
else:
    p.output_files = [ 'tdb.root' ]
    p.histogram_file = 'dqmMonitoringFile.root'
