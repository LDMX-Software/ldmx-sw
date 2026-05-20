#Ricardo,05-26
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('test')

from LDMX.SimCore import simulator as sim

my_sim = sim.Simulator( "my_sim" )
det = 'ldmx-vertTS-v14-8gev'
my_sim.set_detector(det, include_scoring_planes_minimal = True )
from LDMX.SimCore import generators as gen

my_sim.generators.append( gen.single_8gev_e_upstream_tagger() )
my_sim.description = 'Basic test Simulation'

p.sequence = [ my_sim ]

import os
import sys

p.run = int(sys.argv[1])
p.max_events = int(sys.argv[2])
p.output_files = ['events.root']


p.histogram_file = 'hist.root'

# Load the full tracking sequence
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_clusters as ecal_cluster

# Load the ECAL modules
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi_and_reco

# Load the HCAL modules
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions

# Load the TS modules
from LDMX.TrigScint.trig_scint import (
        TrigScintClusterProducer,
        TrigScintDigiProducer,
        trig_scint_track
)


ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]

ts_clusters = [
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        ]
    
trig_scint_track.delta_vert_max=1. #0 or 1
#0 creates tracks with the same centroid
#1 allows for tracks with a deviation in the last pad
# one centroid to the left or to the right

# In order to run this geometry, one have 
# to manually define these quantities
trig_scint_track.number_horizontal_bars = 16
trig_scint_track.horizontal_bar_gap = 2.1
trig_scint_track.number_vertical_bars = 8 
trig_scint_track.vertical_bar_gap = 0.1

# Load the DQM modules
from LDMX.DQM import dqm

# Load electron counting and trigger
from LDMX.Recon.electron_counter import ElectronCounter



count = ElectronCounter(
    simulated_electron_number=1,
    instance_name="ElectronCounter",
    input_pass_name="",
)

p.logger.term_level = 0

p.sequence.extend([
        *ts_digis,
        *ts_clusters,
        trig_scint_track,
        count, 
        ])
