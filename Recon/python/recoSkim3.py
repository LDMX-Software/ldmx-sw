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
from LDMX.Recon.simpleSeqTrigger import simpleSeqTrigger


p.run = 1
p.inputFiles =[ "simoutput.root" ]

simpleSeqTrigger.trigger_list = ["Trigger","Trigger2"]
ORList = [i for i in range(1,2**(len(simpleSeqTrigger.trigger_list)+1))]
ANDList = [sum([2**i for i in range(len(simpleSeqTrigger.trigger_list))])]
simpleSeqTrigger.trigger_passName = thisPassName
simpleSeqTrigger.pass_mask = ORList
simpleSeqTrigger.doOR = False
simpleSeqTrigger.doAND = True

#BDT seems to crash
#p.sequence = [ecalReDigi, ecalReReco, ecalRerecoVeto, tsDigisTag, tsDigisUp, tsDigisDown, tsClustersTag, tsClustersUp, tsClustersDown, trigScintTrack, eCount, simpleTrig1, simpleTrig2, simpleSeqTrigger, hcalReDigi, hcalReReco] 
p.sequence = [simpleSeqTrigger]
p.outputFiles= [ "output3.root" ]
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


