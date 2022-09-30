#!/usr/bin/python

import sys
import os
import json

# we need the ldmx configuration package to construct the object

from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
# the other two pass names refer to those of the input files which will be combined in this job.

thisPassName="reconSeq" #overlay" 
simPassName="sim"
nElectrons=1
p=ldmxcfg.Process( thisPassName )

#import all processors

# Ecal hardwired/geometry stuff
from LDMX.Ecal import EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
egeom = EcalGeometry.EcalGeometryProvider.getInstance()

#Hcal hardwired/geometry stuff                                                                                                                    
from LDMX.Hcal import HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
hgeom = HcalGeometry.HcalGeometryProvider.getInstance()

from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor
#from LDMX.Recon.simpleTrigger import simpleTrigger as simpleTrig2
from LDMX.Recon.simpleSeqTrigger import simpleSeqTrigger



p.run = 1
p.inputFiles =[ "input.root" ]
eCount = ElectronCounter( nElectrons, "ElectronCounter") # first argument is number of electrons in simulation 
eCount.use_simulated_electron_number = True #False
eCount.input_pass_name=simPassName

#trigger skim
simpleTrig1 = TriggerProcessor("YOLO")
simpleTrig1.start_layer= 0   #make sure it doesn't start from 1 (old default bug)
simpleTrig1.input_pass=simPassName
simpleTrig1.thresholds = [ 2750.0, 1000. + simpleTrig1.beamEnergy, 500. + 2*simpleTrig1.beamEnergy, 100. + 3*simpleTrig1.beamEnergy ]

#simpleTrig2.start_layer= 0   #make sure it doesn't start from 1 (old default bug)

simpleTrig2 = TriggerProcessor("YOLO2")
simpleTrig2.input_pass=simPassName
simpleTrig2.beamEnergy = 4000.

simpleTrig2.thresholds = [ 3750.0, 1000. + simpleTrig2.beamEnergy, 500. + 2*simpleTrig2.beamEnergy, 100. + 3*simpleTrig2.beamEnergy ]  #toy something up  

simpleTrig2.trigger_collection = "Trigger2"
simpleTrig2.start_layer=0

simpleSeqTrigger.trigger_list = ["Trigger","Trigger2"]
ORList = [i for i in range(1,2**(len(simpleSeqTrigger.trigger_list)+1))]
ANDList = [sum([2**i for i in range(len(simpleSeqTrigger.trigger_list))])]
simpleSeqTrigger.trigger_passName = thisPassName
simpleSeqTrigger.pass_mask = ANDList

#BDT seems to crash
#p.sequence = [ecalReDigi, ecalReReco, ecalRerecoVeto, tsDigisTag, tsDigisUp, tsDigisDown, tsClustersTag, tsClustersUp, tsClustersDown, trigScintTrack, eCount, simpleTrig1, simpleTrig2, simpleSeqTrigger, hcalReDigi, hcalReReco] 
p.sequence = [eCount, simpleTrig1, simpleTrig2]
p.outputFiles= [ "simoutput.root" ]
p.termLogLevel = 0  # default is 2 (WARNING); but then logFrequency is ignored. level 1 = INFO.                                                            
#print this many events to stdout (independent on number of events, edge case: round-off effects when not divisible. so can go up by a factor 2 or so)    
logEvents=20
if p.maxEvents < logEvents :
     logEvents = p.maxEvents
p.logFrequency = int( p.maxEvents/logEvents )

# if it's not set, it's because we're doing pileup, right, and expect on order 10k events per job
if not p.maxEvents: 
     p.logFrequency = 1000

json.dumps(p.parameterDump(), indent=2)

with open('parameterDump.json', 'w') as outfile:
     json.dump(p.parameterDump(),  outfile, indent=4)


