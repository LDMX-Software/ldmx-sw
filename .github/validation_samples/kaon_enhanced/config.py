import os
import sys

from LDMX.Framework import ldmxcfg

thisPassName = 'test'
p=ldmxcfg.Process(thisPassName)

# This need to be set but has no meaning given p.totalEvents
p.maxEvents = 1
# kaon-focused sample takes about twice as long as normal PN,
# so we ask for half as many events such that validation jobs stay
# time-limited by the PN
p.totalEvents = int(os.environ['LDMX_NUM_EVENTS']) // 2
p.run = int(os.environ['LDMX_RUN_NUMBER'])

from LDMX.SimCore import generators as gen
from LDMX.SimCore import bias_operators
from LDMX.SimCore import kaon_physics
from LDMX.SimCore import photonuclear_models as pn
from LDMX.Biasing import ecal
from LDMX.Biasing import filters
from LDMX.Biasing import particle_filter
from LDMX.Biasing import util
from LDMX.Biasing import include as includeBiasing

detector = 'ldmx-det-v15-8gev'
generator=gen.single_8gev_e_upstream_tagger()
bias_factor = 550.
bias_treshold = 5000.

mySim = ecal.photo_nuclear(detector, generator)
mySim.description = f'8 GeV ECal Kaon PN simulation, xsec bias {bias_factor}' 
mySim.biasing_operators = [ bias_operators.PhotoNuclear('ecal',bias_factor,bias_treshold,only_children_of_primary = True) ]

# Configure the sequence in which user actions should be called.
includeBiasing.library()
mySim.actions.clear()
mySim.actions.extend([
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
mySim.kaon_parameters = kaon_physics.KaonPhysics.upKaons()

# Alternative pn models
myModel = pn.BertiniAtLeastNProductsModel.kaon() 
# Count all (not stopped) particles as "hard"
myModel.hard_particle_threshold=0.
# Apply the model to any nucleus
myModel.zmin = 0
# Apply the model for photonuclear reactions with > 5000 MeV photons
myModel.emin = 5000.
# PDG ids for K^0_L, K^0_S, K^0, K^+, and K^- respectively
myModel.pdg_ids = [130, 310, 311, 321, -321]
# Require at least 1 hard particle from the list above
myModel.min_products = 1

# Change the default model to the kaon producing model
mySim.photonuclear_model = myModel

# Add the filter at the end of the current list of user actions. 
# Filter for events with a kaon daughter
myFilter = particle_filter.PhotoNuclearProductsFilter.kaon()
mySim.actions.extend([myFilter])

p.sequence=[mySim]

# Load the full tracking sequance
from LDMX.Tracking import full_tracking_sequence

# Load the ECAL modules
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
from LDMX.Ecal import digi as eDigi
from LDMX.Ecal import vetos as ecal_vetos

# Load the HCAL modules
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
from LDMX.Hcal import digi as hDigi
from LDMX.Hcal import hcal

# Load the TS modules                                                                    
from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack

# Load electron counting and trigger
from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

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
ecalReco   =eDigi.EcalRecProducer()
ecalDigi = eDigi.EcalDigiProducer()
ecalVeto = ecal_vetos.EcalVetoProcessor()
ecalMip = ecal_vetos.EcalMipProcessor()
ecal_veto_pnet = ecal_vetos.EcalPnetVetoProcessor()

# HCAL part
hcal_digi_reco = hDigi.HcalSimpleDigiAndRecProducer()

# electron counter for trigger processor 
eCount = ElectronCounter( 1, "ElectronCounter") # first argument is number of electrons in simulation
eCount.input_pass_name = ''
simpleTrig = TriggerProcessor("simpleTrig",8000.)
simpleTrig.input_pass=thisPassName

# Load DQM 
from LDMX.DQM import dqm

# Load HCAL veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()

p.logger.termLevel = 1

# Add full tracking for both tagger and recoil trackers: digi, seeds, CFK, ambiguity resolution, GSF, DQM
p.sequence.extend(full_tracking_sequence.sequence)
p.sequence.extend(full_tracking_sequence.dqm_sequence)

p.sequence.extend([ 
        ecalDigi, 
        ecalReco, 
        ecalVeto,
        ecalMip,
        ecal_veto_pnet,
        *ts_digis,
        *ts_clusters, 
        trigScintTrack, 
        eCount, 
        simpleTrig, 
        hcal_digi_reco,
        hcal_veto,
        dqm.PhotoNuclearDQM()
        ])

p.sequence.extend(dqm.all_dqm)


p.histogramFile = f'hist.root'
p.outputFiles = [f'events.root']
