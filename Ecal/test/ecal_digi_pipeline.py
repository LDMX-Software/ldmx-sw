#!/bin/python3

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("v12")
p.run = 1

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions

from LDMX.SimCore import simulator
sim = simulator.simulator("mySim")
sim.setDetector( 'ldmx-det-v12', True  )
sim.description = "ECal Digi Pipeline Tested on Basic 4GeV Gun"
from LDMX.SimCore import generators
sim.generators = [ generators.single_4gev_e_upstream_tagger() ]
sim.generators[0].direction = [ 0. , 0. , 1. ] #straight at ecal
sim.generators[0].position  = [ 0. , 0. , 235. ] #right in front of ecal
sim.beamSpotSmear = [80., 80., 0]
from LDMX.Ecal import digi
from LDMX.Ecal import ecal_trig_digi
from LDMX.DQM import dqm
p.sequence=[ sim, 
        digi.EcalDigiProducer(),
        ecal_trig_digi.EcalTrigPrimDigiProducer(),
        digi.EcalRecProducer(), 
        dqm.EcalDigiVerify()
        ]
p.outputFiles=['ecal_digi_pipeline.root']
p.histogramFile = 'ecal_digi_verify_hists.root'
p.maxEvents = 1000

