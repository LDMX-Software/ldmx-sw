#!/bin/python3

import os
import sys
import json

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("v12")
p.run = 1

from LDMX.SimCore import simulator
sim = simulator.simulator("mySim")
sim.setDetector( 'ldmx-det-v12', True  )
sim.description = "ECal photo-nuclear, xsec bias 450"
sim.randomSeeds = [ 2*p.run , 2*p.run+1 ]
sim.beamSpotSmear = [20., 80., 0]
from LDMX.SimCore import generators
sim.generators = [ generators.single_4gev_e_upstream_tagger() ]
sim.biasingOn(True)
sim.biasingConfigure('photonNuclear', 'ecal', 2500., 450)
from LDMX.Biasing import filters
sim.actions = [ filters.TaggerVetoFilter(),
                filters.TargetBremFilter(),
                filters.EcalProcessFilter(), 
                filters.TrackProcessFilter.photo_nuclear() ]

from LDMX.EventProc.trigScint import TrigScintDigiProducer
from LDMX.EventProc.trigScint import TrigScintClusterProducer
from LDMX.EventProc.trigScint import trigScintTrack
tsDigisUp   = TrigScintDigiProducer.up()
tsDigisTag  = TrigScintDigiProducer.tagger()
tsDigisDown = TrigScintDigiProducer.down()
clTag=TrigScintClusterProducer.tagger()
clUp=TrigScintClusterProducer.up()
clDown=TrigScintClusterProducer.down()

from LDMX.Ecal import digi
from LDMX.Ecal import vetos
from LDMX.EventProc import hcal
from LDMX.EventProc.simpleTrigger import simpleTrigger 
from LDMX.EventProc.trackerHitKiller import trackerHitKiller
p.sequence=[ sim, 
        digi.EcalDigiProducer(),
        digi.EcalRecProducer(), 
        vetos.EcalVetoProcessor(),
        hcal.HcalDigiProducer(),
        hcal.HcalVetoProcessor(), 
        tsDigisUp, tsDigisTag, tsDigisDown,
        clTag, clUp, clDown,
        trigScintTrack,
        trackerHitKiller, simpleTrigger, 
        ldmxcfg.Producer('finableTrack','ldmx::FindableTrackProcessor','EventProc'),
        ldmxcfg.Producer('trackerVeto' ,'ldmx::TrackerVetoProcessor'  ,'EventProc')
        ]

p.outputFiles=["/tmp/simoutput.root"]
p.maxEvents = 1000
with open('/tmp/parameterDump.json', 'w') as outfile:
     json.dump(p.parameterDump(),  outfile, indent=4)

