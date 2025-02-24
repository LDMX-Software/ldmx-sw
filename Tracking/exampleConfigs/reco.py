# Example of jobOption to run tracking on input simulated files in ldmx-sw
# For detailed description of the various configurations, check the .py module files inside
# Tracking/python

import os,math
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

p = ldmxcfg.Process("TrackerReco")

# Load the tracking module
from LDMX.Tracking import tracking

# This has to stay after defining the "TrackerReco" Process in order to load the geometry
# From the conditions
from LDMX.Tracking import geo

n_evts=1000  #number of events to gen/reco

#   set up a simple particle gun for this example  #
#   just 8gev electrons started upstream of tagger and first ts #
partGunString='single_8gev_e_upstream_tagger'
detector = 'ldmx-det-v14-8gev-no-cals'
####  set up beam simulation
sim = simulator.simulator('inclusive_single_8gev')
sim.setDetector(detector,include_scoring_planes = True)  
sim.description = 'A single 8gev electron shot from upstream of the 8gev tagger.'
sim.beamSpotSmear = [20., 80., 0]
particle_gun = generators.single_8gev_e_upstream_tagger()
sim.generators.append(particle_gun)
p.sequence = [sim]
####  end beam simulation

# Load the full tracking sequance
from LDMX.Tracking import full_tracking_sequence

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity resolution, GSF, DQM
p.sequence.extend(full_tracking_sequence.sequence)

# Output name
#   just append '_withTracking' to the name of the input file
from pathlib import Path
p.outputFiles = ['test_8gev_electrons_withTracking.root']

# lower log level so 'info' and above messages can be printed
p.termLogLevel=1

# Number of events
p.maxEvents = n_evts

# Where to store DQM plots
p.histogramFile = "test_dqmMonitoringFile.root"

