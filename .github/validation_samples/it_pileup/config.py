#!/usr/bin/python

import sys
import os

from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
simPassName="ecal_pn"
pileupFilePassName="pileup"
thisPassName="overlay"
p=ldmxcfg.Process(thisPassName)

det = 'ldmx-det-v15-8gev'
p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS']) // 2

# Load the full tracking sequance
from LDMX.Tracking import full_tracking_sequence

from LDMX.Recon.overlay import OverlayProducer
overlay=OverlayProducer('pileup.root')
overlay.sim_passname = simPassName                  #sim input event pass name
overlay.overlay_passname = pileupFilePassName    #pileup input event pass name

p.sequence = [overlay]

# ECal geometry nonsense
from LDMX.Ecal import EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions

# Hcal hardwired/geometry stuff
from LDMX.Hcal import HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions

from LDMX.Ecal import digi as eDigi
from LDMX.Ecal import vetos as ecal_vetos
import LDMX.Ecal.ecalClusters as ecal_cluster
from LDMX.Hcal import digi as hDigi

# this is hardwired into the code to be appended to the sim hits collections
overlayStr="Overlay"

# Load the TS modules
from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack

ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]
for digi in ts_digis :
    digi.input_collection += overlayStr

ts_clusters = [
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        ] 
for clu in ts_clusters :
    clu.input_pass_name = thisPassName

trigScintTrack.input_pass_name = thisPassName


# Load the ECAL modules                           
ecalDigi   = eDigi.EcalDigiProducer('ecalDigis')
ecalReco   = eDigi.EcalRecProducer('ecalRecon')
ecalVeto   = ecal_vetos.EcalVetoProcessor('ecalVetoBDT')
ecalMip = ecal_vetos.EcalMipProcessor('ecalMip')

# The newly produced, overlayed simhits
ecalDigi.inputCollName += overlayStr
ecalDigi.inputPassName = thisPassName

# Use the digis produced above
ecalReco.digiPassName = thisPassName
# SimHits are used to find noise
ecalReco.simHitCollName += overlayStr
ecalReco.simHitPassName = thisPassName

ecalVeto.recoil_from_tracking = False
ecalVeto.rec_pass_name = thisPassName

# Load the HCAL modules
import LDMX.Hcal.digi as hcal_digi_and_reco
hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()
# The newly produced, overlayed simhits
hcal_digi.input_coll_name  += overlayStr
hcal_digi.input_pass_name = thisPassName
hcal_digi.sim_hit_pass_name = thisPassName
# Use the digis produced above
hcal_reco.input_pass_name = thisPassName
hcal_reco.sim_hit_pass_name = thisPassName

# Load ElectronCounter and Trigger
from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(2,'ElectronCounter')
count.input_pass_name = thisPassName

# Load HCAL veto
import LDMX.Hcal.hcal as hcal
hcal_veto = hcal.HcalVetoProcessor()
hcal_veto.input_hit_pass_name = thisPassName

# Load and configure  particle flow sequence. 
# Here we use PF "tracking" and CLUE Ecal clustering 
from LDMX.Recon import pfReco
trackPF = pfReco.pfTrackProducer()
trackPF.inputTrackCollName=trackPF.inputTrackCollName+overlayStr #"EcalScoringPlaneHitsOverlay" #                                                                                                                   
trackPF.input_pass_name=thisPassName
trackPF.doElectronTracking=True
# reference info
truthPF = pfReco.pfTruthProducer()
    
# CLUE     
import LDMX.Ecal.ecalClusters as cl
cluster = cl.EcalClusterProducer()
cluster.seed_threshold = 350. 
cluster.dc = 0.3
cluster.nbr_of_layers = 1
cluster.reclustering = True                                                                                                                                                                                
cluster.rec_hit_pass_name=thisPassName #run on process+pileup       

# particle flow:
pfComb=pfReco.pfProducer()
pfComb.inputEcalCollName = cluster.cluster_coll_name # use CLUE                                                                                                                                                 
pfComb.input_ecal_passname = thisPassName
# trigger recasting existing CLUE to caloclusters
pfComb.use_existing_ecal_clusters = True 

# Load pileup finder
from LDMX.Recon import pileupFinder
puFinder = pileupFinder.pileupFinder()
puFinder.rec_hit_pass_name=thisPassName
#needs recast caloclusters, not (CLUE) ecalclusters 
puFinder.cluster_coll_name=pfComb.inputEcalCollName+"Cast"                                                                                                                   
puFinder.pf_cand_coll_name=pfComb.outputCollName
puFinder.min_momentum=3000.

# Load the DQM modules
from LDMX.DQM import dqm

trigScint_sim_dqm = [
    dqm.TrigScintSimDQM('TrigScintSimPad1','TriggerPad1SimHits','pad1'),
    dqm.TrigScintSimDQM('TrigScintSimPad2','TriggerPad2SimHits','pad2'),
    dqm.TrigScintSimDQM('TrigScintSimPad3','TriggerPad3SimHits','pad3'),
    ]

for ts_sim_dqm in trigScint_sim_dqm :
    ts_sim_dqm.hit_collection += overlayStr
 
trigScint_dqm = [
    dqm.TrigScintDigiDQM('TrigScintDigiPad1','trigScintDigisPad1','pad1'),
    dqm.TrigScintDigiDQM('TrigScintDigiPad2','trigScintDigisPad2','pad2'),
    dqm.TrigScintDigiDQM('TrigScintDigiPad3','trigScintDigisPad3','pad3'),
    dqm.TrigScintClusterDQM('TrigScintClusterPad1','TriggerPad1Clusters','pad1'),
    dqm.TrigScintClusterDQM('TrigScintClusterPad2','TriggerPad2Clusters','pad2'),
    dqm.TrigScintClusterDQM('TrigScintClusterPad3','TriggerPad3Clusters','pad3'),
    dqm.TrigScintTrackDQM('TrigScintTracks','TriggerPadTracks')
    ]

for ts_dqm in trigScint_dqm :
    ts_dqm.passName = thisPassName

# EcalDigiVerify
ecalDigiVerify = dqm.EcalDigiVerify()
ecalDigiVerify.ecal_sim_hit_coll += overlayStr

# EcalShowerFeatures
ecalShowerFeatures = dqm.EcalShowerFeatures()
ecalShowerFeatures.ecal_veto_pass = thisPassName

# EcalMipTrackingFeatures
ecalMipTrackingFeatures = dqm.EcalMipTrackingFeatures()
ecalMipTrackingFeatures.ecal_veto_pass = thisPassName

# EcalVetoResults
ecalVetoResults = dqm.EcalVetoResults()
ecalVetoResults.ecal_veto_pass = thisPassName
ecal_veto_pnet =  ecal_vetos.EcalPnetVetoProcessor()
ecal_veto_pnet.ecal_rec_hits_passname = thisPassName

# HCAL DQM
hcalDQM = [
    dqm.HCalDQM(pe_threshold=8,
                section=0
                ),
    dqm.HCalDQM(pe_threshold=8,
                section=1
                ),
    dqm.HCalDQM(pe_threshold=8,
                section=2
                ),
    dqm.HCalDQM(pe_threshold=8,
                section=3
                ),
    dqm.HCalDQM(pe_threshold=8,
                section=4
                ),
    dqm.HcalInefficiencyAnalyzer(),
]

for hdqm in hcalDQM:
    hdqm.rec_pass_name = thisPassName
    hdqm.sim_pass_name = thisPassName
    hdqm.sim_coll_name += overlayStr

# Trigger DQM
triggerDQM = dqm.Trigger()
triggerDQM.trigger_pass = thisPassName


dqm_with_overlay = trigScint_sim_dqm + trigScint_dqm + [triggerDQM, ecalDigiVerify, ecalShowerFeatures, ecalMipTrackingFeatures, ecalVetoResults] + hcalDQM 

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
    hcal_digi,
    hcal_reco,
    hcal_veto,
    *ts_digis,
    *ts_clusters,
    trigScintTrack,
    count, TriggerProcessor('trigger', 8000.),
    dqm.PhotoNuclearDQM()
])

p.sequence.extend(dqm_with_overlay)

# Add PFlow + pileup finding sequence 
p.sequence.extend([
     cluster,
     dqm.EcalClusterAnalyzer(),
     trackPF,
     truthPF,
     pfComb,        
     puFinder
])

p.inputFiles = ['ecal_pn.root']
p.outputFiles= ['events.root']
p.histogramFile = 'hist.root'
