# Example of jobOption to run tracking on input simulated files in ldmx-sw
# For detailed description of the various configurations, check the .py module files
# inside
# Tracking/python

import os
import math
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

p = ldmxcfg.Process("TrackerReco")

# Load the tracking module
from LDMX.Tracking import tracking

# This has to stay after defining the "TrackerReco" Process in order to load the
# geometry
# From the conditions
from LDMX.Tracking import geo

n_evts = 1000  # number of events to gen/reco

#   set up a simple particle gun for this example  #
#   just 8gev electrons started upstream of tagger and first ts #
part_gun_string = "single_8gev_e_upstream_tagger"
detector = "ldmx-det-v14-8gev-no-cals"
####  set up beam simulation
sim = simulator.Simulator("inclusive_single_8gev")
sim.set_detector(detector, include_scoring_planes_minimal=True)
sim.description = "A single 8gev electron shot from upstream of the 8gev tagger."
particle_gun = generators.single_8gev_e_upstream_tagger()
sim.generators = [particle_gun]
p.sequence = [sim]
####  end beam simulation

# Load the full tracking sequence
from LDMX.Tracking.full_tracking_sequence import full_tracking_sequence

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity
# resolution, GSF, DQM
trk = full_tracking_sequence(detector=detector)
p.sequence.extend(trk.sequence)
p.sequence.extend(trk.dqm_sequence)

# Output name
#   just append '_withTracking' to the name of the input file
from pathlib import Path

p.output_files = ["test_8gev_electrons_withTracking.root"]

# lower log level so 'info' and above messages can be printed
# p.term_log_level = 1

# Number of events
p.max_events = n_evts

# Where to store DQM plots
p.histogram_file = "test_dqmMonitoringFile.root"
