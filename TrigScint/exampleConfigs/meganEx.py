#!/bin/python

import sys
import os
import json

# we need the ldmx configuration package to construct the object

from LDMX.Framework import ldmxcfg

# set a 'pass name'
pass_name="sim"
p=ldmxcfg.Process(pass_name)

#import all processors
from LDMX.SimCore import generators
from LDMX.SimCore import simulator
from LDMX.Biasing import filters

from LDMX.Detectors.makePath import *
from LDMX.SimCore import simcfg

#pull in command line options
n_ele=4      # simulated beam electrons
run_num=10
version="ldmx-det-v14"
output_name_string= "ldmxdetv14gap10mm.root" #sample identifier
out_dir= ""    #sample identifier

#
# Instantiate the simulator.
#
sim = simulator.simulator("test")

#
# Set the path to the detector to use (pulled from job config)
#
sim.setDetector( version, include_scoring_planes_minimal = True )
sim.scoringPlanes = makeScoringPlanesPath(version)

outname=output_name_string #+".root"
print("NAME = " + outname)

#
# Set run parameters. These are all pulled from the job config
#
p.run = run_num
p.max_events = 100
n_electrons = n_ele
beam_energy = 4.0  #in GeV

sim.description = "Inclusive "+str(beam_energy)+" GeV electron events, "+str(n_electrons)+"e"
#sim.randomSeeds = [ SEED1 , SEED2 ]
sim.beamSpotSmear = [20., 80., 0]


mpg_gen = generators.multi( "mgpGen" ) # this is the line that actually creates the generator
mpg_gen.vertex = [ -44., 0., -880. ] # mm
mpg_gen.nParticles = n_electrons
mpg_gen.pdgID = 11
mpg_gen.enablePoisson = False #True

import math
theta = math.radians(5.45)
beam_energy_mev=1000*beam_energy
px = beam_energy_mev*math.sin(theta)
py = 0.
pz= beam_energy_mev*math.cos(theta)
mpg_gen.momentum = [ px, py, pz ]

#
# Set the multiparticle gun as generator
#
sim.generators = [ mpg_gen ]

#reconstruction and vetoes

#Ecal and Hcal hardwired/geometry stuff
#import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
from LDMX.Ecal import EcalGeometry
#egeom = EcalGeometry.EcalGeometryProvider.getInstance()
#Hcal hardwired/geometry stuff
from LDMX.Hcal import HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
#hgeom = HcalGeometry.HcalGeometryProvider.getInstance()


from LDMX.Ecal import digi as eDigi
from LDMX.Ecal import vetos
from LDMX.Hcal import digi as hDigi
from LDMX.Hcal import hcal

from LDMX.Recon.simple_trigger import TriggerProcessor

from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trig_scint_track

if "v12" in version :
     ts_sim_colls=[ "TriggerPadTagSimHits", "TriggerPadUpSimHits", "TriggerPadDnSimHits" ]
else :
     ts_sim_colls=[ "TriggerPad2SimHits", "TriggerPad3SimHits", "TriggerPad1SimHits" ]

# ecal digi chain
# ecal_digi   =eDigi.EcalDigiProducer('EcalDigis')
# ecal_reco   =eDigi.EcalRecProducer('ecalRecon')
# ecal_veto   =vetos.EcalVetoProcessor('ecalVetoBDT')

# #hcal digi chain
# hcalDigi   =hDigi.HcalDigiProducer('hcal_digis')
# hcalReco   =hDigi.HcalRecProducer('hcalRecon')
# hcalVeto   =hcal.HcalVetoProcessor('hcalVeto')
# #hcalDigi.inputCollName="HcalSimHits"
#hcalDigi.input_pass_name=pass_name

# TS digi + clustering + track chain
ts_digis_tag =TrigScintDigiProducer.pad2()
ts_digis_tag.input_collection  = ts_sim_colls[0]# +"_"+pass_name
ts_digis_tag.input_pass_name = "sim"
ts_digis_up  =TrigScintDigiProducer.pad3()
ts_digis_up.input_collection   = ts_sim_colls[1]# +"_"+pass_name
ts_digis_up.input_pass_name = "sim"
ts_digis_down=TrigScintDigiProducer.pad1()
ts_digis_down.input_collection = ts_sim_colls[2]# +"_"+pass_name
ts_digis_down.input_pass_name = "sim"

ts_clusters_tag  =TrigScintClusterProducer.pad2()
ts_clusters_up  =TrigScintClusterProducer.pad1()
ts_clusters_down  =TrigScintClusterProducer.pad3()

if "v12" in version :
     ts_clusters_tag.pad_time = -2.
     ts_clusters_up.pad_time = 0.
     ts_clusters_down.pad_time = 0.

ts_digis_up.verbosity=0
ts_clusters_up.verbosity=1
trig_scint_track.verbosity=1

trig_scint_track.delta_max = 0.75

from LDMX.Recon.electron_counter import ElectronCounter
e_count = ElectronCounter( n_electrons, "ElectronCounter") # first argument is number of electrons in simulation
e_count.use_simulated_electron_number = False
e_count.input_collection="TriggerPadTracks"
e_count.input_pass_name=pass_name

# # p.sequence=[ sim, ecal_digi, ecal_reco, ecal_veto, hcalDigi, hcalReco, hcalVeto, ts_digis_tag, ts_digis_up, ts_digis_down, ts_clusters_tag, ts_clusters_up, ts_clusters_down, trig_scint_track, e_count ]
# #hcal digi keeps crashing in config step
p.sequence=[ sim, ts_digis_tag, ts_digis_up, ts_digis_down, ts_clusters_tag, ts_clusters_up, ts_clusters_down, trig_scint_track, e_count]
# p.sequence=[sim]

p.output_files=[outname]

p.term_log_level = 0  # default is 2 (WARNING); but then logFrequency is ignored. level 1 = INFO.

#print this many events to stdout (independent on number of events, edge case: round-off effects when not divisible. so can go up by a factor 2 or so)
log_events=20
if p.max_events < log_events :
     log_events = p.max_events
p.log_frequency = int( p.max_events/log_events )

json.dumps(p.parameter_dump(), indent=2)

with open('parameterDump.json', 'w') as outfile:
     json.dump(p.parameter_dump(),  outfile, indent=4)
