#!/usr/bin/python

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("drop")

p.libraries.append("libEventProc.so")

from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.EventProc.ecalDigis import ecalDigis

p.keep = [ 
            "keep .*SimHits.*" #keep all SimHits
            , "drop .*EcalSimHits.*" #except those in the ECal
         ]

p.sequence=[ hcalDigis, ecalDigis ]

p.inputFiles=["input.root"]

p.maxEvents=50

p.outputFiles=["output.root"]

p.printMe()

