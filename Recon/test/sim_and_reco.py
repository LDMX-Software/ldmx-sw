#!/bin/python3

import json
import os
import sys

from LDMX.Framework import ldmxcfg


p=ldmxcfg.Process("v12")
p.run = 1

import LDMX.Ecal.ecal_geometry
from LDMX.SimCore import simulator


sim = simulator.Simulator("my_sim")
sim.set_detector( 'ldmx-det-v12', include_scoring_planes_minimal = True  )
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

from LDMX.TrigScint.trigScint import (
     TrigScintClusterProducer,
     TrigScintDigiProducer,
     trig_scint_track,
)


ts_digis_up   = TrigScintDigiProducer.up()
ts_digis_tag  = TrigScintDigiProducer.tagger()
ts_digis_down = TrigScintDigiProducer.down()
cl_tag=TrigScintClusterProducer.tagger()
cl_up=TrigScintClusterProducer.up()
cl_down=TrigScintClusterProducer.down()

from LDMX.Ecal import digi, ecal_hardcoded_conditions, vetos
from LDMX.Hcal import hcal
from LDMX.Recon.simple_trigger import simple_trigger


#from LDMX.EventProc.trackerHitKiller import trackerHitKiller
p.sequence=[ sim,
        digi.EcalDigiProducer(),
        digi.EcalRecProducer(),
        vetos.EcalVetoProcessor(),
        hcal.HcalDigiProducer(),
        hcal.HcalVetoProcessor(),
        ts_digis_up, ts_digis_tag, ts_digis_down,
        cl_tag, cl_up, cl_down,
        trig_scint_track,
        #trackerHitKiller,
        simple_trigger,
        #ldmxcfg.Producer('finableTrack','ldmx::FindableTrackProcessor','EventProc'),
        #ldmxcfg.Producer('trackerVeto' ,'ldmx::TrackerVetoProcessor'  ,'EventProc')
        ]

p.output_files=["/tmp/simoutput.root"]
p.max_events = 1000
with open('/tmp/parameterDump.json', 'w') as outfile:
     json.dump(p.parameter_dump(),  outfile, indent=4)

