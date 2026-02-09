from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process( "PF" )
import sys


p.max_events = 10
if len(sys.argv) > 1 :
    p.max_events = int(sys.argv[1])

# we want to see every event
p.logFrequency = 1 if p.max_events <= 10 else 100
p.termLogLevel = 1

# Set a run number
p.run = 9001

# we also only have an output file
p.outputFiles = [ "pfReco_" + str(p.max_events) + "_events.root" ]

import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
from LDMX.SimCore import simulator as sim


my_sim = sim.simulator( "my_sim" )
my_sim.setDetector( 'ldmx-det-v14' , include_scoring_planes_minimal = True )
sim.beamSpotSmear = [20., 80., 0.]

# Get a pre-written generator
from particleSources import cocktail_commands

from LDMX.SimCore import generators as gen


my_sim.generators.append( gen.gps( 'my_gps' , cocktail_commands) )
# add your configured simulation to the sequence
p.sequence.append( my_sim )

# reco stuff

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
from LDMX.TrigScint.trigScint import (
    TrigScintClusterProducer,
    TrigScintDigiProducer,
    trig_scint_track,
)


ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]
for d in ts_digis :
    d.randomSeed = 1

from LDMX.DQM import dqm
from LDMX.Recon.electron_counter import ElectronCounter
from LDMX.Recon.simple_trigger import TriggerProcessor


count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

from LDMX.Ecal import ecal_trig_digi
from LDMX.Hcal import hcal_trig_digi


p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        ])

if True: #False:
    p.setCompression(2, level=9) # LZMA
    from LDMX.Recon import pfReco
    ecal_pf = pfReco.pfEcalClusterProducer()
    hcal_pf = pfReco.pfHcalClusterProducer()
    track_pf = pfReco.pfTrackProducer()
    truth_pf = pfReco.pfTruthProducer()


    # configure clustering options
    ecal_pf.doSingleCluster = False
    ecal_pf.logEnergyWeight = True

    hcal_pf.doSingleCluster = False
    hcal_pf.clusterHitDist = 200. # mm
    hcal_pf.logEnergyWeight = True

    ecalPF_simple = pfReco.pfEcalClusterProducer()
    ecalPF_simple.clusterCollName += "Simple"
    ecalPF_simple.doSingleCluster = True
    hcalPF_simple = pfReco.pfHcalClusterProducer()
    hcalPF_simple.clusterCollName += "Simple"
    hcalPF_simple.doSingleCluster = True

    p.sequence.extend([
        ecal_pf, hcal_pf, track_pf,
        pfReco.pfProducer(),
        truth_pf,
        #ecalPF_simple, hcalPF_simple
    ])


