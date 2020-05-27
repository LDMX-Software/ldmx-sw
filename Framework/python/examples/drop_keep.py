#!/usr/bin/python

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("drop")

p.libraries.append("libEventProc.so")
p.libraries.append("libEcal.so")

from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.Ecal.ecalDigis import ecalDigis
from LDMX.Ecal.ecalRecon import ecalRecon

p.keep = [ 
            "drop .*SimHits.*" #drop all sim hits
            ,"keep EcalSimHits.*" #except the ones in the ECal
         ]

p.sequence=[ hcalDigis, ecalDigis, ecalRecon ]

p.inputFiles=["input.root"] #CHANGE ME to your actual input file

p.maxEvents=50

p.outputFiles=["output.root"]

print p

