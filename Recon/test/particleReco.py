from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process( "PF" )
import sys
p.maxEvents = 10
if len(sys.argv) > 1 :
    p.maxEvents = int(sys.argv[1])

# we want to see every event
p.logFrequency = 1 if p.maxEvents <= 10 else 100
p.termLogLevel = 1

# Set a run number
p.run = 9001

# we also only have an output file
p.outputFiles = [ "pfReco_" + str(p.maxEvents) + "_events.root" ]

from LDMX.SimCore import simulator as sim
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v14' , True )
sim.beamSpotSmear = [20., 80., 0.]

# Get a pre-written generator
from LDMX.SimCore import generators as gen

from particleSources import cocktail_commands
mySim.generators.append( gen.gps( 'myGPS' , cocktail_commands) )
# add your configured simulation to the sequence
p.sequence.append( mySim )

# reco stuff

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi

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

from LDMX.DQM import dqm

from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

from LDMX.Hcal import hcal_trig_digi
from LDMX.Ecal import ecal_trig_digi

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        ])

if True: #False:
    p.setCompression(2, level=9) # LZMA                                                                                                                              
    from LDMX.Recon import pfReco
    ecalPF = pfReco.pfEcalClusterProducer()
    hcalPF = pfReco.pfHcalClusterProducer()
    trackPF = pfReco.pfTrackProducer()
    truthPF = pfReco.pfTruthProducer()
    
    
    # configure clustering options
    ecalPF.doSingleCluster = False
    ecalPF.logEnergyWeight = True
    
    hcalPF.doSingleCluster = False
    hcalPF.clusterHitDist = 200. # mm
    hcalPF.logEnergyWeight = True
    
    ecalPF_simple = pfReco.pfEcalClusterProducer()
    ecalPF_simple.clusterCollName += "Simple"
    ecalPF_simple.doSingleCluster = True
    hcalPF_simple = pfReco.pfHcalClusterProducer()
    hcalPF_simple.clusterCollName += "Simple"
    hcalPF_simple.doSingleCluster = True
    
    p.sequence.extend([
        ecalPF, hcalPF, trackPF,
        pfReco.pfProducer(),
        truthPF,
        #ecalPF_simple, hcalPF_simple
    ])


