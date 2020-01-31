#!/usr/bin/python

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("drop")

p.libraries.append("libEventProc.so")

from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.EventProc.ecalDigis import ecalDigis

p.keep = [ 
        "ignore .*_sim" #ignore everything from the sim pass
        , "drop .*SimHits.*" #read in SimHits but don't write them out
         ]

p.sequence=[ hcalDigis, ecalDigis ]

p.inputFiles=["input.root"]

p.maxEvents=50

p.outputFiles=["output.root"]

p.printMe()

