#!/usr/bin/python

import sys
import os

from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
simPassName="test"
pileupFilePassName="test"
thisPassName="overlay"
p=ldmxcfg.Process(thisPassName)

det = 'ldmx-det-v14-8gev'
p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])
p.termLogLevel = 0

from LDMX.Recon.overlay import OverlayProducer
overlay=OverlayProducer('pileup.root')
overlay.passName = simPassName                  #sim input event pass name
overlay.overlayPassName = pileupFilePassName    #pileup input event pass name
overlay.totalNumberOfInteractions = 2.
overlay.doPoissonIntime = False
overlay.doPoissonOutoftime = False
overlay.nEarlierBunchesToSample = 0
overlay.bunchSpacing = 26.9      #ns = 1000./37.2 ; 5.4 = 1000./186.
overlay.timeSpread = 0.       # <-- realistically, 30 ps; 
overlay.timeMean   = 0.       # <-- here set the in-bunch average pu time offset to no time shift whatsoever; useful for validation though
overlay.overlayCaloHitCollections=[ 
        "TriggerPad1SimHits", "TriggerPad2SimHits", "TriggerPad3SimHits", "TargetSimHits", 
        "EcalSimHits", "HcalSimHits"]
overlay.overlayTrackerHitCollections=[ "TaggerSimHits", "RecoilSimHits" ]
overlay.verbosity=0 #1 #3 #

p.sequence = [overlay]

# ECal geometry nonsense
from LDMX.Ecal import EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions

# Hcal hardwired/geometry stuff
from LDMX.Hcal import HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions

from LDMX.Ecal import digi as eDigi
from LDMX.Ecal import vetos
from LDMX.Hcal import digi as hDigi

overlayStr="Overlay"

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
    d.input_collection += overlayStr

                                                            
ecalDigi   =eDigi.EcalDigiProducer('ecalDigis')
ecalReco   =eDigi.EcalRecProducer('ecalRecon')
ecalVeto   =vetos.EcalVetoProcessor('ecalVetoBDT')

ecalDigi.inputCollName  = ecalDigi.inputCollName+overlayStr
ecalReco.simHitCollName = ecalReco.simHitCollName+overlayStr
ecalReco.digiPassName = thisPassName
ecalVeto.rec_pass_name = thisPassName

hcalDigi   =hDigi.HcalDigiProducer('hcalDigis')
hcalDigi.inputCollName  = hcalDigi.inputCollName+overlayStr
hcalReco   =hDigi.HcalRecProducer('hcalRecon')
hcalReco.digiPassName = thisPassName

from LDMX.DQM import dqm
ecalDigiVerify = dqm.EcalDigiVerify()
ecalDigiVerify.ecalSimHitColl = ecalDigiVerify.ecalSimHitColl+overlayStr

from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

p.sequence.extend([
    ecalDigi, ecalReco, ecalVeto,
    hcalDigi, hcalReco,
    *ts_digis,
    TrigScintClusterProducer.pad1(),
    TrigScintClusterProducer.pad2(),
    TrigScintClusterProducer.pad3(),
    trigScintTrack,
    count, TriggerProcessor('trigger', 8000.),
    dqm.SimObjects(sim_pass=thisPassName),
    ecalDigiVerify,dqm.EcalShowerFeatures(),
        dqm.PhotoNuclearDQM(verbose=False),
    dqm.HCalDQM(),

]+dqm.recoil_dqm+dqm.trigger_dqm)

p.inputFiles = ['ecal_pn.root']
p.outputFiles= ['events.root']
p.histogramFile = 'hist.root'
