import sys

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("test")
p.libraries.append("libEventProc.so")
detIDChecker = ldmxcfg.Producer("ecalDigis","ldmx::DetIDAnalyzer")
p.sequence=[detIDChecker]
p.inputFiles=["input.root"]
p.printMe()
