#!/usr/bin/python

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("drop")

p.libraries.append("libEventProc.so")
p.libraries.append("libEcal.so")

from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.Ecal.ecalDigis import ecalDigis
from LDMX.Ecal.ecalRecon import ecalRecon

p.keep = [ 
            "keep .*SimHits.*" #keep all SimHits
            , "drop .*EcalSimHits.*" #except those in the ECal
         ]

p.sequence=[ hcalDigis, ecalDigis, ecalRecon ]

p.inputFiles=["input.root"] #CHANGE ME to your actual input file

p.maxEvents=50

p.outputFiles=["output.root"]

print p

