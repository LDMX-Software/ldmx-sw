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

from LDMX.Detectors.make_path import make_scoring_planes_path
from LDMX.SimCore import simcfg

#pull in command line options
n_ele=4      # simulated beam electrons
run_num=10
version="ldmx-det-v14"
output_name_string= "ldmxdetv14gap10mm_firmware.root" #sample identifier
out_dir= ""    #sample identifier

#
# Instantiate the simulator.
#
sim = simulator.Simulator("test")

#
# Set the path to the detector to use (pulled from job config)
#
sim.set_detector( version, include_scoring_planes_minimal = True )
sim.scoring_planes = make_scoring_planes_path(version)

outname=output_name_string #+".root"
print("NAME = " + outname)

#
# Set run parameters. These are all pulled from the job config
#
p.run = run_num
p.max_events = 100
n_electrons = n_ele
beam_energy = 4.0  #in GeV

sim.description = (
    "Inclusive "+str(beam_energy)+" GeV electron events, "+str(n_electrons)+"e"
)
#sim.randomSeeds = [ SEED1 , SEED2 ]
sim.beamSpotSmear = [20., 80., 0]


# this is the line that actually creates the generator
mpg_gen = generators.Multi( "mgpGen" )
mpg_gen.vertex = [ -44., 0., -880. ] # mm
mpg_gen.n_particles = n_electrons
mpg_gen.pdg_id = 11
mpg_gen.enable_poisson = False #True

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
#import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions
from LDMX.Ecal import ecal_geometry
#egeom = EcalGeometry.EcalGeometryProvider.get_instance()
#Hcal hardwired/geometry stuff
from LDMX.Hcal import hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions
#hgeom = HcalGeometry.HcalGeometryProvider.get_instance()


from LDMX.Ecal import digi as ecal_digi
from LDMX.Ecal import vetos
from LDMX.Hcal import digi as hcal_digi
from LDMX.Hcal import hcal

from LDMX.Recon.simple_trigger import TriggerProcessor

from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trig_scint_track
from LDMX.TrigScint.trigScint import TrigScintFirmwareTracker

ts_sim_colls=[ "TriggerPad2SimHits", "TriggerPad3SimHits", "TriggerPad1SimHits" ]

# ecal digi chain
# ecal_digi   =ecal_digi.EcalDigiProducer('EcalDigis')
# ecal_reco   =ecal_digi.EcalRecProducer('ecalRecon')
# ecal_veto   =vetos.EcalVetoProcessor('ecalVetoBDT')

# #hcal digi chain
# hcalDigi   =hcal_digi.HcalDigiProducer('hcal_digis')
# hcalReco   =hcal_digi.HcalRecProducer('hcalRecon')
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


ts_digis_up.verbosity=0
ts_clusters_up.verbosity=1
trig_scint_track.verbosity=1

trig_scint_track.delta_max = 0.75

trig_firm = TrigScintFirmwareTracker( "trig_firm" )
trig_firm.input_pass_name = "sim"
trig_firm.digis1_collection = "trigScintDigisPad1"
trig_firm.digis2_collection = "trigScintDigisPad2"
trig_firm.digis3_collection = "trigScintDigisPad3"
trig_firm.output_collection = "TriggerPadTracksFirmware"

from LDMX.Recon.electron_counter import ElectronCounter
e_count = ElectronCounter(
    simulated_electron_number=n_electrons,
    instance_name="ElectronCounter",
    use_simulated_electron_number=False,
    input_collection="TriggerPadTracks",
    input_pass_name=pass_name,
)

from LDMX.TrigScint.trigScint import TrigScintFirmwareHitProducer
from LDMX.TrigScint.trigScint import TrigScintQIEDigiProducer
from LDMX.TrigScint.trigScint import TrigScintRecHitProducer

qie_digi = TrigScintQIEDigiProducer.pad3()
rechit = TrigScintRecHitProducer.pad3()
hit_firm = TrigScintFirmwareHitProducer( "hit_firm" )
hit_firm.verbose = True



# # p.sequence=[ sim, ecal_digi, ecal_reco, ecal_veto, hcalDigi, hcalReco, hcalVeto,
# ts_digis_tag, ts_digis_up, ts_digis_down, ts_clusters_tag, ts_clusters_up,
# ts_clusters_down, trig_scint_track, e_count ]
# #hcal digi keeps crashing in config step
p.sequence = [
    sim, ts_digis_tag, ts_digis_up, ts_digis_down,
    ts_clusters_tag, ts_clusters_up, ts_clusters_down,
    trig_scint_track, trig_firm, e_count, qie_digi, rechit, hit_firm,
]
# p.sequence=[sim]

p.output_files=[outname]

# default is 2 (WARNING); but then logFrequency is ignored. level 1 = INFO.
p.term_log_level = 0

# print this many events to stdout (independent on number of events,
# edge case: round-off
#effects when not divisible. so can go up by a factor 2 or so)
log_events=20
if p.max_events < log_events :
     log_events = p.max_events
p.log_frequency = int( p.max_events/log_events )

json.dumps(p.parameter_dump(), indent=2)

with open('parameterDump.json', 'w') as outfile:
     json.dump(p.parameter_dump(),  outfile, indent=4)
