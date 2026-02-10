#!/bin/python3

from LDMX.Framework import ldmxcfg


p=ldmxcfg.Process("v12")
p.run = 1

import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.ecal_geometry
from LDMX.SimCore import simulator


sim = simulator.simulator("my_sim")
sim.setDetector( 'ldmx-det-v12', include_scoring_planes_minimal = True  )
sim.description = "ECal Digi Pipeline Tested on Basic 4GeV Gun"
from LDMX.SimCore import generators


sim.generators = [ generators.single_4gev_e_upstream_tagger() ]
sim.generators[0].direction = [ 0. , 0. , 1. ] #straight at ecal
sim.generators[0].position  = [ 0. , 0. , 235. ] #right in front of ecal
sim.beamSpotSmear = [80., 80., 0]
from LDMX.DQM import dqm
from LDMX.Ecal import digi, ecal_trig_digi


p.sequence=[ sim,
        digi.EcalDigiProducer(),
        ecal_trig_digi.EcalTrigPrimDigiProducer(),
        digi.EcalRecProducer(),
        dqm.EcalDigiVerify()
        ]
p.output_files=['ecal_digi_pipeline.root']
p.histogram_file = 'ecal_digi_verify_hists.root'
p.max_events = 1000

