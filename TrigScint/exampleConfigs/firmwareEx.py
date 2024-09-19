#!/bin/python

import sys
import os
import json

# we need the ldmx configuration package to construct the object

from LDMX.Framework import ldmxcfg

# set a 'pass name'
passName="sim"
p=ldmxcfg.Process(passName)

#import all processors
from LDMX.SimCore import generators
from LDMX.SimCore import simulator
from LDMX.Biasing import filters

from LDMX.Detectors.makePath import *
from LDMX.SimCore import simcfg

#pull in command line options
nEle=4      # simulated beam electrons
runNum=10
version="ldmx-det-v14"
outputNameString= "ldmxdetv14gap10mm_firmware.root" #sample identifier
outDir= ""    #sample identifier

#
# Instantiate the simulator.
#
sim = simulator.simulator("test")

#
# Set the path to the detector to use (pulled from job config)
#
sim.setDetector( version, True )
sim.scoringPlanes = makeScoringPlanesPath(version)

outname=outputNameString #+".root"
print("NAME = " + outname)

#
# Set run parameters. These are all pulled from the job config 
#
p.run = runNum
p.maxEvents = 100
nElectrons = nEle
beamEnergy = 4.0;  #in GeV                                                                                                                                              

sim.description = "Inclusive "+str(beamEnergy)+" GeV electron events, "+str(nElectrons)+"e"
#sim.randomSeeds = [ SEED1 , SEED2 ]
sim.beamSpotSmear = [20., 80., 0]


mpgGen = generators.multi( "mgpGen" ) # this is the line that actually creates the generator                                                                  
mpgGen.vertex = [ -44., 0., -880. ] # mm                                                                                                                              
mpgGen.nParticles = nElectrons
mpgGen.pdgID = 11
mpgGen.enablePoisson = False #True                                                                                                                                      

import math
theta = math.radians(5.45)
beamEnergyMeV=1000*beamEnergy
px = beamEnergyMeV*math.sin(theta)
py = 0.;
pz= beamEnergyMeV*math.cos(theta)
mpgGen.momentum = [ px, py, pz ]

#
# Set the multiparticle gun as generator
#
sim.generators = [ mpgGen ]
         
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

from LDMX.Recon.simpleTrigger import TriggerProcessor

from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack
from LDMX.TrigScint.trigScint import TrigScintFirmwareTracker

tsSimColls=[ "TriggerPad2SimHits", "TriggerPad3SimHits", "TriggerPad1SimHits" ]

# ecal digi chain
# ecalDigi   =eDigi.EcalDigiProducer('EcalDigis')
# ecalReco   =eDigi.EcalRecProducer('ecalRecon')
# ecalVeto   =vetos.EcalVetoProcessor('ecalVetoBDT')

# #hcal digi chain
# hcalDigi   =hDigi.HcalDigiProducer('hcalDigis')
# hcalReco   =hDigi.HcalRecProducer('hcalRecon')                  
# hcalVeto   =hcal.HcalVetoProcessor('hcalVeto')
# #hcalDigi.inputCollName="HcalSimHits"
#hcalDigi.inputPassName=passName

# TS digi + clustering + track chain
tsDigisTag =TrigScintDigiProducer.pad2()
tsDigisTag.input_collection  = tsSimColls[0]# +"_"+passName
tsDigisTag.input_pass_name = "sim"
tsDigisUp  =TrigScintDigiProducer.pad3()
tsDigisUp.input_collection   = tsSimColls[1]# +"_"+passName
tsDigisUp.input_pass_name = "sim"
tsDigisDown=TrigScintDigiProducer.pad1()
tsDigisDown.input_collection = tsSimColls[2]# +"_"+passName
tsDigisDown.input_pass_name = "sim"

tsClustersTag  =TrigScintClusterProducer.pad2()
tsClustersUp  =TrigScintClusterProducer.pad1()
tsClustersDown  =TrigScintClusterProducer.pad3()


tsDigisUp.verbosity=0
tsClustersUp.verbosity=1
trigScintTrack.verbosity=1

trigScintTrack.delta_max = 0.75 

trigFirm = TrigScintFirmwareTracker( "trigFirm" )
trigFirm.input_pass_name = "sim"
trigFirm.digis1_collection = "trigScintDigisPad1"
trigFirm.digis2_collection = "trigScintDigisPad2"
trigFirm.digis3_collection = "trigScintDigisPad3"
trigFirm.output_collection = "TriggerPadTracksFirmware"

from LDMX.Recon.electronCounter import ElectronCounter
eCount = ElectronCounter( nElectrons, "ElectronCounter") # first argument is number of electrons in simulation 
eCount.use_simulated_electron_number = False
eCount.input_collection="TriggerPadTracks"
eCount.input_pass_name=passName

# # p.sequence=[ sim, ecalDigi, ecalReco, ecalVeto, hcalDigi, hcalReco, hcalVeto, tsDigisTag, tsDigisUp, tsDigisDown, tsClustersTag, tsClustersUp, tsClustersDown, trigScintTrack, eCount ]
# #hcal digi keeps crashing in config step
p.sequence=[ sim, tsDigisTag, tsDigisUp, tsDigisDown, tsClustersTag, tsClustersUp, tsClustersDown, trigScintTrack, trigFirm, eCount]
# p.sequence=[sim]

p.outputFiles=[outname]

p.termLogLevel = 0  # default is 2 (WARNING); but then logFrequency is ignored. level 1 = INFO.

#print this many events to stdout (independent on number of events, edge case: round-off effects when not divisible. so can go up by a factor 2 or so)
logEvents=20 
if p.maxEvents < logEvents :
     logEvents = p.maxEvents
p.logFrequency = int( p.maxEvents/logEvents )

#json.dumps(p.parameterDump(), indent=2)

with open('parameterDump.json', 'w') as outfile:
     json.dump(p.parameterDump(),  outfile, indent=4)
