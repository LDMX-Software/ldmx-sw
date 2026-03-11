import os
import sys

from LDMX.Framework import ldmxcfg


this_pass_name = 'test'
p=ldmxcfg.Process(this_pass_name)

# This need to be set but has no meaning given p.total_events
p.max_events = 1
# kaon-focused sample takes about twice as long as normal PN,
# so we ask for half as many events such that validation jobs stay
# time-limited by the PN
p.total_events = int(os.environ['LDMX_NUM_EVENTS']) // 2
p.run = int(os.environ['LDMX_RUN_NUMBER'])

from LDMX.Biasing import ecal, filters, particle_filter, util
from LDMX.Biasing import include as include_biasing
from LDMX.SimCore import bias_operators, kaon_physics
from LDMX.SimCore import generators as gen
from LDMX.SimCore import photonuclear_models as pn


detector = 'ldmx-det-v15-8gev'
generator=gen.single_8gev_e_upstream_tagger()
bias_factor = 550.
bias_treshold = 5000.

my_sim = ecal.photo_nuclear(detector, generator)
my_sim.description = f'8 GeV ECal Kaon PN simulation, xsec bias {bias_factor}'
my_sim.biasing_operators = [ bias_operators.PhotoNuclear('ecal',bias_factor,bias_treshold,only_children_of_primary = True) ]

# Configure the sequence in which user actions should be called.
include_biasing.library()
my_sim.actions.clear()
my_sim.actions.extend([
        filters.TaggerVetoFilter(thresh=2*3800.),
        # Only consider events where a hard brem occurs
        filters.TargetBremFilter(recoil_max_p = 2*1500., brem_min_e = 2*2500.),
        # Only consider events where a PN reaction happnes in the ECal
        filters.EcalProcessFilter(),
        # Tag all photo-nuclear tracks to persist them to the event.
        util.TrackProcessFilter.photo_nuclear()
])

# set up "upKaon" parameters which reduces the charged kaon lifetimes by a factor 1/50
# and forces decays to be into one of the leptonic decay modes.
my_sim.kaon_parameters = kaon_physics.KaonPhysics.upKaons()

# Alternative pn models
my_model = pn.BertiniAtLeastNProductsModel.kaon()
# Count all (not stopped) particles as "hard"
my_model.hard_particle_threshold = 0.
# Apply the model to any nucleus
my_model.zmin = 0
# Apply the model for photonuclear reactions with > 5000 MeV photons
my_model.emin = 5000.
# PDG ids for K^0_L, K^0_S, K^0, K^+, and K^- respectively
my_model.pdg_ids = [130, 310, 311, 321, -321]
# Require at least 1 hard particle from the list above
my_model.min_products = 1

# Change the default model to the kaon producing model
my_sim.photonuclear_model = my_model

# Add the filter at the end of the current list of user actions.
# Filter for events with a kaon daughter
my_filter = particle_filter.PhotoNuclearProductsFilter.kaon()
my_sim.actions.extend([my_filter])

p.sequence=[my_sim]

# Load the full tracking sequance
# Load the ECAL modules
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions

# Load the HCAL modules
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions
from LDMX.Ecal import digi as eDigi
from LDMX.Ecal import vetos as ecal_vetos
from LDMX.Hcal import digi as hDigi
from LDMX.Hcal import hcal

# Load electron counting and trigger
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor
from LDMX.Tracking import full_tracking_sequence

# Load the TS modules
from LDMX.TrigScint.trig_scint import (
        TrigScintClusterProducer,
        TrigScintDigiProducer,
        trig_scint_track,
)


#TS digi + clustering + track chain
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

# ECAL part
ecal_reco = eDigi.EcalRecProducer()
ecal_digi = eDigi.EcalDigiProducer()
ecal_veto = ecal_vetos.EcalVetoProcessor()
ecal_mip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet = ecal_vetos.EcalPnetVetoProcessor()

# HCAL part
import LDMX.Hcal.digi as hcal_digi_and_reco


hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()

# electron counter for trigger processor
# first argument is number of electrons in simulation
e_count = ElectronCounter(simulated_electron_number=1, instance_name="ElectronCounter", input_pass_name='')
simple_trig = TriggerProcessor(instance_name="simple_trig", beam_energy=8000., input_pass=this_pass_name)

# Load DQM
from LDMX.DQM import dqm


# Load HCAL veto
hcal_veto = hcal.HcalVetoProcessor()

p.logger.term_level = 1
# Example to show trace level logging for sim (only)
# p.logger.custom("KaonPhysics", level = -1)


# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity resolution, GSF, DQM
p.sequence.extend(full_tracking_sequence.sequence)
p.sequence.extend(full_tracking_sequence.dqm_sequence)

p.sequence.extend([
        ecal_digi,
        ecal_reco,
        ecal_veto,
        ecal_mip,
        ecal_veto_pnet,
        *ts_digis,
        *ts_clusters,
        trig_scint_track,
        e_count,
        simple_trig,
        hcal_digi,
        hcal_reco,
        hcal_veto,
        dqm.PhotoNuclearDQM()
        ])

p.sequence.extend(dqm.all_dqm)


p.histogram_file = 'hist.root'
p.output_files = ['events.root']
